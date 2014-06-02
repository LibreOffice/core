/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_TABLEAUTOFORMATFIELD_HXX
#define INCLUDED_TEST_SHEET_TABLEAUTOFORMATFIELD_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

namespace apitest {

class TableAutoFormatField
{
public:
    //don't use virtual init() here
    css::uno::Reference< css::beans::XPropertySet > initTest();

    virtual css::uno::Reference< > getServiceFactory() = 0;

    void testRotateReference();
    void testVertJustify();
};

}

#endif // INCLUDED_TEST_SHEET_TABLEAUTOFORMATFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
