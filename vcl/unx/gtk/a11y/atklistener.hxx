/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atklistener.hxx,v $
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

#ifndef _ATK_LISTENER_HXX_
#define _ATK_LISTENER_HXX_

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/implbase1.hxx>

#include <vector>

#include "atkwrapper.hxx"

typedef std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > AccessibleVector;

class AtkListener : public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleEventListener >
{
public:
    AtkListener(AtkObjectWrapper * pWrapper);

    // XEventListener
    virtual void disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventListener
    virtual void notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException );

    AtkObject *mpAccessible;
    AccessibleVector m_aChildList;

private:

    virtual ~AtkListener();

    // Updates the child list held to provide the old IndexInParent on children_changed::remove
    void updateChildList(::com::sun::star::accessibility::XAccessibleContext* pContext);

    // Process CHILD_EVENT notifications with a new child added
    void handleChildAdded(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& rxParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxChild);

    // Process CHILD_EVENT notifications with a child removed
    void handleChildRemoved(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& rxParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxChild);

    // Process INVALIDATE_ALL_CHILDREN notification
    void handleInvalidateChildren(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& rxParent);
};

#endif /* _ATK_LISTENER_HXX_ */

