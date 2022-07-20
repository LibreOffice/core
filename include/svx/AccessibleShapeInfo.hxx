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

#include <com/sun/star/uno/Reference.hxx>
#include <svx/svxdllapi.h>

namespace com::sun::star {
    namespace accessibility { class XAccessible; }
    namespace drawing { class XShape; }
}

namespace accessibility {

class IAccessibleParent;

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
    css::uno::Reference<
        css::drawing::XShape> mxShape;

    /** The accessible parent object of the shape.
    */
    css::uno::Reference<
        css::accessibility::XAccessible> mxParent;

    /** This object that may be realized by the same implementation as that
        of that of <member>mxParent</member> can be used to modify
        parent/child relationships with the shape as initiator.
        Note that NULL is a valid value for this member.
    */
    IAccessibleParent* mpChildrenManager;

    /** Copy the given values into the members described above.
    */
    AccessibleShapeInfo (
        css::uno::Reference<
            css::drawing::XShape> xShape,
        css::uno::Reference<
            css::accessibility::XAccessible> xParent,
        IAccessibleParent* pChildrenManager);

    /** Copy the given values into the members described above.
        The accessible parent implementation object is set to NULL.
    */
    AccessibleShapeInfo (
        css::uno::Reference<
            css::drawing::XShape> xShape,
        css::uno::Reference<
            css::accessibility::XAccessible> xParent);

    ~AccessibleShapeInfo();

    AccessibleShapeInfo (const AccessibleShapeInfo&);
private:
    AccessibleShapeInfo& operator= (const AccessibleShapeInfo&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
