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

#ifndef INCLUDED_VCL_INC_OSX_A11YLISTENER_HXX
#define INCLUDED_VCL_INC_OSX_A11YLISTENER_HXX

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/implbase.hxx>

#include "a11yfocustracker.hxx"
#include "osxvcltypes.h"
#include <set>
#include <com/sun/star/awt/Rectangle.hpp>

// - AquaA11yEventListener -

class AquaA11yEventListener :
    public ::cppu::WeakImplHelper< css::accessibility::XAccessibleEventListener >
{

public:
    AquaA11yEventListener(id wrapperObject, sal_Int16 role);
    virtual ~AquaA11yEventListener();

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
        throw( css::uno::RuntimeException, std::exception ) override;

private:
    const id m_wrapperObject;
    const sal_Int16 m_role;
    css::awt::Rectangle m_oldBounds;
};

#endif // INCLUDED_VCL_INC_OSX_A11YLISTENER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
