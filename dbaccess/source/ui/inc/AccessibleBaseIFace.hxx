/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_ACCESSIBLE_HELPER_IFACE_HXX
#define DBAUI_ACCESSIBLE_HELPER_IFACE_HXX

#include <sal/types.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
