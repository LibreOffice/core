/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_UNOAPIPROPERTYHELPER_HXX
#define INCLUDED_TEST_SHEET_UNOAPIPROPERTYHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <sal/config.h>
#include <test/testdllapi.hxx>

namespace apitest
{
/** @brief Tester for test property type 'boolean' of a @see com::sun::star::beans::XPropertySet.
 *
 * @param   xPropertySet    The property set, which contains the property to test against.
 * @param   name            Name of property to test.
 */
void OOO_DLLPUBLIC_TEST testBooleanProperty(
    css::uno::Reference<css::beans::XPropertySet>& xPropertySet, const OUString& name);

/** @brief Tester for test property type 'long' of a @see com::sun::star::beans::XPropertySet.
 *
 * @param   xPropertySet    The property set, which contains the property to test against.
 * @param   name            Name of property to test.
 * @param   nValue          Value to use when setting a new value.
 */
void OOO_DLLPUBLIC_TEST
testLongProperty(css::uno::Reference<css::beans::XPropertySet>& xPropertySet, const OUString& name,
                 const sal_Int32& nValue);

/** @brief Tester for property type 'short' of a @see com::sun::star::beans::XPropertySet.
 *
 * @param   xPropertySet    The property set, which contains the property to test against.
 * @param   name            Name of property to test.
 * @param   nValue          Value to use when setting a new value.
 */
void OOO_DLLPUBLIC_TEST
testShortProperty(css::uno::Reference<css::beans::XPropertySet>& xPropertySet, const OUString& name,
                  const sal_Int16& nValue);

/** @brief Tester for property type 'string' of a @see com::sun::star::beans::XPropertySet.
 *
 * @param   xPropertySet    The property set, which contains the property to test against.
 * @param   name            Name of property to test.
 * @param   rValue          Value to use when setting a new value.
 */
void OOO_DLLPUBLIC_TEST
testStringProperty(css::uno::Reference<css::beans::XPropertySet>& xPropertySet,
                   const OUString& name, const OUString& rValue);
}
#endif // INCLUDED_TEST_SHEET_UNOAPIPROPERTYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
