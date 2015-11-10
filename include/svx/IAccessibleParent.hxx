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

#ifndef INCLUDED_SVX_IACCESSIBLEPARENT_HXX
#define INCLUDED_SVX_IACCESSIBLEPARENT_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <sal/types.h>
#include "AccessibleControlShape.hxx"
namespace accessibility {

class AccessibleShape;
class AccessibleShapeTreeInfo;

/** This interface contains methods missing from the
    XAccessibleContext interface that allow the modification of
    parent/child relationship.
*/
class IAccessibleParent
{
public:
    /** Allow for a virtual destructor.
    */
    virtual ~IAccessibleParent(){};

    /** A call to this method requests the implementor to replace one child
        with another and send the appropriate notifications.  That are two
        child events: One notifying the removal of the current child and one
        about the existence of the new child.  The index of the new child is
        implementation dependent, i.e. it is not guaranteed that the
        replacement has the same index as the current child has.

        <p>A default implementation can just use the ShapeTypeHandler::CreateAccessibleObject
        to let a factory create the new instance with the parameters given, and then
        place the new shape into the own structures.</p>

        @param pCurrentChild
            This child is about to be replaced.

        @param _rxShape
            The UNO shape which the old and new child represent

        @param _nIndex
            The IndexInParent of the old child. Note that the index in
            parent of the replacement is not necessarily the same as
            that of the current child.

        @param _rShapeTreeInfo
            The TreeInfo for the old child.

        @return
           If the replacement has taken place successfully <TRUE/> is
           returned.  If the replacement can not be carried out or an error
           occurs that does not result in an exception then <FALSE/> is
           returned.

        @raises RuntimeException
           in case something went heavily wrong
    */
    virtual bool ReplaceChild (
        AccessibleShape* pCurrentChild,
        const css::uno::Reference< css::drawing::XShape >& _rxShape,
        const long _nIndex,
        const AccessibleShapeTreeInfo& _rShapeTreeInfo
    )   throw (css::uno::RuntimeException) = 0;
    //Add this method to support Form Controls
    virtual AccessibleControlShape* GetAccControlShapeFromModel
        (css::beans::XPropertySet*)
        throw (css::uno::RuntimeException){return nullptr;};
    virtual  css::uno::Reference<
            css::accessibility::XAccessible>
        GetAccessibleCaption (const css::uno::Reference<
            css::drawing::XShape>&)
            throw (css::uno::RuntimeException){return nullptr;};
    virtual bool IsDocumentSelAll(){ return false; }
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
