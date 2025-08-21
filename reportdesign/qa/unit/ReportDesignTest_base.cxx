/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/unoapi_test.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XCloseable.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class ReportDesignTestBase : public UnoApiTest
{
public:
    ReportDesignTestBase()
        : UnoApiTest(u"reportdesign/qa/unit/data"_ustr){};

    void loadURLCopy(std::u16string_view rURL);
    virtual void tearDown() override;
};

void ReportDesignTestBase::loadURLCopy(std::u16string_view sURL)
{
    createTempCopy(sURL);
    mxComponent = mxDesktop->loadComponentFromURL(maTempFile.GetURL(), u"_default"_ustr, 0, {});
}

void ReportDesignTestBase::tearDown()
{
    if (mxComponent)
    {
        // In order to close all windows
        css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
        xCloseable->close(false);
    }

    UnoApiTest::tearDown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
