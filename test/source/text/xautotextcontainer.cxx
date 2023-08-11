/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/xautotextcontainer.hxx>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/text/XAutoTextContainer.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XAutoTextContainer::testInsertNewByName()
{
    uno::Reference<text::XAutoTextContainer> xAutoTextContainer(init(), uno::UNO_QUERY_THROW);

    try
    {
        xAutoTextContainer->removeByName(m_aName + "Insert");
    }
    catch (container::NoSuchElementException&)
    {
    }
    uno::Reference<text::XAutoTextGroup> xGroup
        = xAutoTextContainer->insertNewByName(m_aName + "Insert");

    CPPUNIT_ASSERT(xGroup.is());
}

void XAutoTextContainer::testRemoveByName()
{
    uno::Reference<text::XAutoTextContainer> xAutoTextContainer(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xAutoTextContainer->hasByName(m_aName));
    xAutoTextContainer->removeByName(m_aName);
    CPPUNIT_ASSERT(!xAutoTextContainer->hasByName(m_aName));
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
