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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETABBARBASE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETABBARBASE_HXX

#include <comphelper/accessiblecomponenthelper.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

class TabBar;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;


namespace accessibility
{




typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleExtendedComponentHelper_BASE;

class AccessibleTabBarBase : public AccessibleExtendedComponentHelper_BASE
{
public:
    explicit            AccessibleTabBarBase( TabBar* pTabBar );
    virtual             ~AccessibleTabBarBase();

protected:
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

    virtual void        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

private:
    void                SetTabBarPointer( TabBar* pTabBar );
    void                ClearTabBarPointer();

protected:
    VCLExternalSolarLock* m_pExternalLock;
    VclPtr<TabBar>        m_pTabBar;
};




}   // namespace accessibility


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
