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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class StatusBar;


//  class VCLXAccessibleStatusBar


class VCLXAccessibleStatusBar : public VCLXAccessibleComponent
{
private:
    typedef ::std::vector< css::uno::Reference< css::accessibility::XAccessible > > AccessibleChildren;

    AccessibleChildren      m_aAccessibleChildren;
    VclPtr<StatusBar>       m_pStatusBar;

protected:
    void                    UpdateShowing( sal_Int32 i, bool bShowing );
    void                    UpdateItemName( sal_Int32 i );
    void                    UpdateItemText( sal_Int32 i );

    void                    InsertChild( sal_Int32 i );
    void                    RemoveChild( sal_Int32 i );

    virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    VCLXAccessibleStatusBar( VCLXWindow* pVCLXWindow );
    virtual ~VCLXAccessibleStatusBar();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
};


#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
