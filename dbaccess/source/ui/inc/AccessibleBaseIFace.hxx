/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleBaseIFace.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:13:30 $
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
#ifndef DBAUI_ACCESSIBLE_HELPER_IFACE_HXX
#define DBAUI_ACCESSIBLE_HELPER_IFACE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace dbaui
{
    class SAL_NO_VTABLE IAccessibleHelper
    {
    protected:
        /** isEditable returns the current editable state
            @return true if it is editable otherwise false
        */
        virtual sal_Bool isEditable() const = 0;
    public:
        /** notifies all listeners that this object has changed
            @param  _nEventId   the event id
            @param  _aOldValue  the old value
            @param  _aNewValue  the new value
        */
        virtual void notifyAccessibleEvent( sal_Int16 _nEventId,
                                            const ::com::sun::star::uno::Any& _aOldValue,
                                            const ::com::sun::star::uno::Any& _aNewValue) = 0;
    };
}
#endif // DBAUI_ACCESSIBLE_HELPER_IFACE_HXX
