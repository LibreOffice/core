/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleShapeInfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:35:26 $
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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_PARENT_HXX
#include <svx/IAccessibleParent.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace accessibility {

/** @descr
        This class is a container for the information specific for a single
        shape that is passed to the constructor of that shape.  It allows to
        separate the class definitions of the shapes from the information
        needed on construction.  Only the shapes' implementation has to be
        adapted to a modified <type>AccessibleShapeInfo</type> definition.

        <p>Note that this class complements the
        <type>AccessibleShapeTreeInfo</type> interface which contains
        information that is passed to all shapes in a subtree not just to a
        single shape. </p>
*/
class SVX_DLLPUBLIC AccessibleShapeInfo
{
public:
    /** The shape itself for which an accessible object is to be
        constructed.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape> mxShape;

    /** The accessible parent object of the shape.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> mxParent;

    /** This object that may be realized by the same implementation as that
        of that of <member>mxParent</member> can be used to modify
        parent/child relationships with the shape as initiator.
        Note that NULL is a valid value for this member.
    */
    IAccessibleParent* mpChildrenManager;

    /** This index is used to disambiguate names of accessible objects. A
        value of (the default) -1 leads to the use of the object's z-order
        instead.  Because that is not a good substitute, better pass an ever
        increasing counter.
    */
    sal_Int32 mnIndex;

    /** Copy the given values into the members described above.
    */
    AccessibleShapeInfo (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        IAccessibleParent* pChildrenManager,
        sal_Int32 nIndex = -1);

    /** Copy the given values into the members described above.
        The accessible parent implementation object is set to NULL.
    */
    AccessibleShapeInfo (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        sal_Int32 nIndex = -1);

    ~AccessibleShapeInfo (void);

    AccessibleShapeInfo (const AccessibleShapeInfo&);
private:
    // Don't use these three methods.
    SVX_DLLPRIVATE explicit AccessibleShapeInfo (void);
    SVX_DLLPRIVATE AccessibleShapeInfo& operator= (const AccessibleShapeInfo&);
};

} // end of namespace accessibility

#endif
