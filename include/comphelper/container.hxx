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

#ifndef INCLUDED_COMPHELPER_CONTAINER_HXX
#define INCLUDED_COMPHELPER_CONTAINER_HXX

#include <vector>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{


//= IndexAccessIterator

/** an iterator that iterates through all elements, starting from an XIndexAccess (pre-order)
*/
class COMPHELPER_DLLPUBLIC IndexAccessIterator
{
protected:
    css::uno::Reference< css::uno::XInterface>    m_xStartingPoint;

    css::uno::Reference< css::uno::XInterface>    m_xCurrentObject;
        // The current object
    ::std::vector<sal_Int32>                      m_arrChildIndizies;

        // I'm moving through a tree, but its elements have no GetNextSibling,
        // so I have to remember where each child is in relation to its parent.
        // That is the path from the root node to m_xCurrentObject

public:
    IndexAccessIterator(css::uno::Reference< css::uno::XInterface> const & xStartingPoint);

    virtual ~IndexAccessIterator();

    css::uno::Reference< css::uno::XInterface> const &    Next();

    virtual void Invalidate() { m_xCurrentObject = nullptr; }

protected:
    virtual bool ShouldHandleElement(const css::uno::Reference< css::uno::XInterface>& /*rElement*/) { return true; }

        // This can be used to exclude certain elements; elements for which
        // this function returns sal_True will be simply skipped.
        // If this element is returned from Next(), then one can get
        // here get a little more information on the element.
        // That's why this method is not const.
    virtual bool ShouldStepInto(const css::uno::Reference< css::uno::XInterface>& /*xContainer*/) const { return true; }
};


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_CONTAINER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
