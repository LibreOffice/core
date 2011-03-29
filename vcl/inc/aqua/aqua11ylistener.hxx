/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _AQUA11YLISTENER_HXX_
#define _AQUA11YLISTENER_HXX_

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/implbase1.hxx>

#include "aqua11yfocustracker.hxx"
#include "aquavcltypes.h"
#include <set>
#include <com/sun/star/awt/Rectangle.hpp>

// -------------------------
// - AquaA11yEventListener -
// -------------------------

class AquaA11yEventListener :
    public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleEventListener >
{

public:
    AquaA11yEventListener(id wrapperObject, sal_Int16 role);
    virtual ~AquaA11yEventListener();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException );

private:
    const id m_wrapperObject;
    const sal_Int16 m_role;
    ::com::sun::star::awt::Rectangle m_oldBounds;
};

#endif // _AQUA11YLISTENER_HXX_