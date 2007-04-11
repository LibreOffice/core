/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IAccessibleViewForwarderListener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:36:49 $
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

#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_LISTENER_HXX
#define _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_LISTENER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif


namespace accessibility {

class IAccessibleViewForwarder;

/** <p>The purpose of this interface is to notify a user of an
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

#endif
