/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletabbarbase.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX

#include <comphelper/accessiblecomponenthelper.hxx>
#include <tools/link.hxx>

class TabBar;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;

//.........................................................................
namespace accessibility
{
//.........................................................................

// ============================================================================

typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleExtendedComponentHelper_BASE;

class AccessibleTabBarBase : public AccessibleExtendedComponentHelper_BASE
{
public:
    explicit            AccessibleTabBarBase( TabBar* pTabBar );
    virtual             ~AccessibleTabBarBase();

protected:
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    virtual void        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing();

private:
    void                SetTabBarPointer( TabBar* pTabBar );
    void                ClearTabBarPointer();

protected:
    VCLExternalSolarLock* m_pExternalLock;
    TabBar*             m_pTabBar;
};

// ============================================================================

//.........................................................................
}   // namespace accessibility
//.........................................................................

#endif

