/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_ACCESSIBLESHAPEINFO_HXX
#define INCLUDED_SVX_ACCESSIBLESHAPEINFO_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <svx/IAccessibleParent.hxx>
#include <svx/svxdllapi.h>

namespace accessibility {

/** @descr
        This class is a container for the information specific for a single
        shape that is passed to the constructor of that shape.  It allows to
        separate the class definitions of the shapes from the information
        needed on construction.  Only the shapes' implementation has to be
        adapted to a modified AccessibleShapeInfo definition.

        <p>Note that this class complements the
        AccessibleShapeTreeInfo interface which contains
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

    ~AccessibleShapeInfo();

    AccessibleShapeInfo (const AccessibleShapeInfo&);
private:
    AccessibleShapeInfo& operator= (const AccessibleShapeInfo&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
