/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atklistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-05 10:56:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _ATK_LISTENER_HXX_
#define _ATK_LISTENER_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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

