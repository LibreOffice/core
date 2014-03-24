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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

#include "AccessibleBrowseBox.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <svtools/accessibletableprovider.hxx>

class SvHeaderTabListBox;



namespace accessibility {

class AccessibleBrowseBoxTable;

typedef ::cppu::ImplHelper1  <   ::com::sun::star::accessibility::XAccessible
                            >   AccessibleTabListBox_Base;

/** !!! */
class AccessibleTabListBox
                :public AccessibleBrowseBox
                ,public AccessibleTabListBox_Base
                ,public ::svt::IAccessibleTabListBox
{
private:
    SvHeaderTabListBox*         m_pTabListBox;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBox(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

public:
    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception );

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception );

    // XAccessibleContext
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException, std::exception );

    // IAccessibleTabListBox
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself()
    {
        return this;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar()
    {
        return AccessibleBrowseBox::getHeaderBar( svt::BBTYPE_COLUMNHEADERBAR );
    }

protected:
    /** dtor() */
    virtual ~AccessibleTabListBox();

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual AccessibleBrowseBoxTable*   createAccessibleTable();
};



} // namespace accessibility



#endif // ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
