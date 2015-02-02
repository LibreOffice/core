/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <svx/svdograf.hxx>

#include "drwlayer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/borderline.hxx>
#include <editeng/flditem.hxx>
#include <dbdata.hxx>
#include "validat.hxx"
#include "formulacell.hxx"
#include "userdat.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "stlsheet.hxx"
#include "docfunc.hxx"
#include "markdata.hxx"
#include "colorscale.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "cellvalue.hxx"
#include "attrib.hxx"
#include "dpshttab.hxx"
#include <scopetools.hxx>
#include <columnspanset.hxx>
#include <tokenstringcontext.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest : public ScBootstrapFixture
{
public:

    ScFiltersTest();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
