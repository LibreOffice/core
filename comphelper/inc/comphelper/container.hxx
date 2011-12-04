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

#ifndef _COMPHELPER_CONTAINER_HXX_
#define _COMPHELPER_CONTAINER_HXX_

#include <vector>
#include "com/sun/star/uno/Reference.hxx"
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

//========================================================================
//= IndexAccessIterator
//========================================================================
/** an iterator that iterates through all elements, starting from an XIndexAccess (pre-order)
*/
class COMPHELPER_DLLPUBLIC IndexAccessIterator
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xStartingPoint;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xCurrentObject;
        // The current object
    ::std::vector<sal_Int32>        m_arrChildIndizies;

        // I'm moving through a tree, but its elements have no GetNextSibling,
        // so I have to remember where each child is in relation to its parent.
        // That is the path from the root node to m_xCurrentObject

    ::rtl::OUString     m_ustrProperty;
        // The Name of the requested property

public:
    IndexAccessIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint);

    virtual ~IndexAccessIterator();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    Next();

    virtual void Invalidate() { m_xCurrentObject = NULL; }

protected:
    virtual sal_Bool ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& /*rElement*/) { return sal_True; }

        // This can be used to exclude certain elements; elements for which
        // this function returns sal_True will be simply skipped.
        // If this element is returned from Next(), then one can get
        // here get a little more information on the element.
        // That's why this method is not const.
    virtual sal_Bool ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& /*xContainer*/) const { return sal_True; }
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_CONTAINER_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
