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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <svx/IAccessibleParent.hxx>
#include "svx/svxdllapi.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
