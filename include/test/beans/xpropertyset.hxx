/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __TEST_BEANS_XPROPERTYSET_HXX__
#define __TEST_BEANS_XPROPERTYSET_HXX__

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include "test/testdllapi.hxx"

#include <vector>

namespace apitest {

class OOO_DLLPUBLIC_TEST XPropertySet
{
public:
    virtual ~XPropertySet();

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    void testGetPropertySetInfo();
    void testAddPropertyChangeListener();
    void testAddVetoableChangeListener();
    void testSetPropertyValue();
    void testGetPropertyValue();
    void testRemovePropertyChangeListener();
    void testRemoveVetoableChangeListener();

protected:
    virtual bool isPropertyValueChangeable(const OUString& rName);

private:
    void fillPropsToTest(const css::uno::Reference<css::beans::XPropertySetInfo>& xPropInfo);
    static bool getSinglePropertyValue(
        const css::uno::Reference<css::beans::XPropertySet>& xPropSet, const OUString& rName);

    struct OOO_DLLPUBLIC_TEST PropsToTest
    {
        std::vector<OUString> constrained;
        std::vector<OUString> bound;
        std::vector<OUString> normal;
        std::vector<OUString> readonly;

        bool initialized;

        PropsToTest();
    };

    PropsToTest maPropsToTest;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
