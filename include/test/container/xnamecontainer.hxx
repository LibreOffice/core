/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_CONTAINER_XNAMECONTAINER_HXX
#define INCLUDED_TEST_CONTAINER_XNAMECONTAINER_HXX

#include <rtl/ustring.hxx>
#include <test/testdllapi.hxx>

#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XNameContainer
{
public:
    // removes default entry
    XNameContainer()
        : m_aName("XNameContainer")
    {
    }
    // removes given entry
    XNameContainer(const OUString& rNameToRemove)
        : m_aName(rNameToRemove)
    {
    }

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual void setElement(const css::uno::Any& rElement) { m_aElement = rElement; }

    void testInsertByName();
    void testInsertByNameEmptyName();
    void testInsertByNameDuplicate();
    void testInsertByNameInvalidElement();
    void testRemoveByName();
    void testRemoveByNameEmptyName();
    void testRemoveByNameNoneExistingElement();

protected:
    ~XNameContainer() {}

private:
    OUString const m_aName;
    css::uno::Any m_aElement;
};

} // namespace apitest

#endif // INCLUDED_TEST_CONTAINER_XNAMECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
