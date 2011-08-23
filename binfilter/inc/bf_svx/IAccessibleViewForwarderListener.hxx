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

#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_LISTENER_HXX
#define _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_LISTENER_HXX

#include <sal/types.h>

namespace binfilter {

namespace accessibility {

class IAccessibleViewForwarder;

/**	<p>The purpose of this interface is to notify a user of an
    IAccessibleViewForwarder when that view forwarder changes its
    properties.  Such a change may be one of the following:
    <ul>
    <li>Change of the coordinate transformation.</li>
    <li>Change of the visible area (which in turn results in a change of the
    coordinate transformation.</li>
    <li>Change of the validity state of the view forwarder.</li>
*/
class IAccessibleViewForwarderListener
{
public:
    /** Enumeration of the different change types.
    */
    enum ChangeType {TRANSFORMATION, VISIBLE_AREA, STATE};

    /** This method is called to indicate a change of the specified view
        forwarder. 
        @param aChangeType
            The type of the change.  TRANSFORMATION indicates a change of
            the coordinate transformation with a constant visible area.  If
            the visible area changes, just use VISIBLE_AREA.  This changes
            the transformation implicitly.  The value STATE indicates a
            change of the validity state.  Check the IsValid method of the
            view forwarder before doing further calls.
        @param pViewForwarder
            The modified view forwarder.  It is specified just in case that
            there is more than one view forwarder in use at the same time.
    */
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder) = 0;
};

} // end of namespace accessibility

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
