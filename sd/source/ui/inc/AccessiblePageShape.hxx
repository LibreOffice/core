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


#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEPAGESHAPE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEPAGESHAPE_HXX

#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/IAccessibleViewForwarderListener.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <svx/AccessibleTextHelper.hxx>

namespace accessibility {

/** A page shape represents the actual page as seen on the screen.
*/
class AccessiblePageShape
    :   public AccessibleShape
{
public:
    //=====  internal  ========================================================

    /** Create a new accessible object that makes the given shape accessible.
        @param rxParent
            The accessible parent object.  It will be used, for example when
            the <member>getIndexInParent</member> method is called.
        @param rShapeTreeInfo
            Bundel of information passed to this shape and all of its desendants.
        @param nIndex
            Index used to disambiguate between objects that have the same
            name.  Passing a value of -1 leads to the use of the object's
            z-order instead.  Because that is not a good substitute, better
            pass an ever increasing counter.
        @attention
            Always call the <member>init</member> method after creating a
            new accessible shape.  This is one way to overcome the potential
            problem of registering the new object with e.g. event
            broadcasters.  That would delete the new object if a broadcaster
            would not keep a strong reference to the new object.
    */
    AccessiblePageShape (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& rxPage,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        const AccessibleShapeTreeInfo& rShapeTreeInfo,
        long nIndex = -1);

    virtual ~AccessiblePageShape (void);

    /** Initialize a new shape.  See the documentation of the constructor
        for the reason of this method's existence.
    */
    virtual void Init (void);

    //=====  XAccessibleContext  ==============================================

    /// Returns always 0 because there can be no children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void)
        throw (std::exception);

    /** Return the specified child.
        @param nIndex
            Index of the requested child.
        @return
            Reference of the requested child which is the accessible object
            of a visible shape.
        @raises IndexOutOfBoundsException
            Throws always an exception because there are no children.
    */
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException, std::exception);


    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    //=====  XComponent  ======================================================

    virtual void SAL_CALL
        dispose (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);


    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);


    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& Source)
        throw (::com::sun::star::uno::RuntimeException, std::exception);


    using AccessibleShape::disposing;

protected:
    /** Create a base name string that contains the accessible name.
    */
    virtual OUString
        CreateAccessibleBaseName (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual OUString
        CreateAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XDrawPage> mxPage;

    /** Don't use the default constructor.  Use the public constructor that
        takes the original shape and the parent as arguments instead.
    */
    explicit AccessiblePageShape (void);
    /// Don't use the copy constructor.  Is there any use for it?
    explicit AccessiblePageShape (const AccessiblePageShape&);
    /// Don't use the assignment operator.  Do we need this?
    AccessibleShape& operator= (const AccessiblePageShape&);
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
