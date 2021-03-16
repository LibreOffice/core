/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

//--
#include <config_folders.h>

#include <stdio.h>
#include <osl/file.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>
#include <sal/log.hxx>

#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

#include <cppuhelper/bootstrap.hxx>

#include <vcl/svapp.hxx>

#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

using namespace ::cppu;
using namespace ::com::sun::star::uno;

using namespace ::com::sun::star;

namespace
{
Reference<awt::XControlModel> importFile(char const* fname,
                                         Reference<XComponentContext> const& xContext)
{
    // create the input stream
    FILE* f = ::fopen(fname, "rb");
    if (f)
    {
        ::fseek(f, 0, SEEK_END);
        int nLength = ::ftell(f);
        ::fseek(f, 0, SEEK_SET);

        std::vector<sal_Int8> bytes(nLength);
        ::fread(bytes.data(), nLength, 1, f);
        ::fclose(f);

        Reference<container::XNameContainer> xModel(
            xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.awt.UnoControlDialogModel", xContext),
            UNO_QUERY);
        ::xmlscript::importDialogModel(::xmlscript::createInputStream(bytes), xModel, xContext);

        return xModel;
    }
    else
    {
        throw Exception("### Cannot read file!");
    }
}

void exportToFile(char const* fname, Reference<awt::XControlModel> const& xModel,
                  Reference<XComponentContext> const& xContext)
{
    Reference<io::XInputStreamProvider> xProvider(::xmlscript::exportDialogModel(xModel, xContext));
    Reference<io::XInputStream> xStream(xProvider->createInputStream());

    Sequence<sal_Int8> bytes;
    sal_Int32 nRead = xStream->readBytes(bytes, xStream->available());
    for (;;)
    {
        Sequence<sal_Int8> readBytes;
        nRead = xStream->readBytes(readBytes, 1024);
        if (!nRead)
            break;

        sal_Int32 nPos = bytes.getLength();
        bytes.realloc(nPos + nRead);
        memcpy(bytes.getArray() + nPos, readBytes.getConstArray(), (sal_uInt32)nRead);
    }

    FILE* f = ::fopen(fname, "w");
    ::fwrite(bytes.getConstArray(), 1, bytes.getLength(), f);
    ::fclose(f);
}

/// Sample tests for import
class XmlScriptTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;
    void testEmptyPopupElements();

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(XmlScriptTest);
    CPPUNIT_TEST(testEmptyPopupElements);
    CPPUNIT_TEST_SUITE_END();
};

void XmlScriptTest::setUp()
{
    //  __debugbreak();
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmlScriptTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void XmlScriptTest::testEmptyPopupElements()
{
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    Reference<lang::XMultiServiceFactory> xMSF(xComponentContext->getServiceManager(), UNO_QUERY);

    ::comphelper::setProcessServiceFactory(xMSF);

    Reference<awt::XToolkit> xToolkit = awt::Toolkit::create(xComponentContext);

    // import dialog
    Reference<awt::XControlModel> xModel(importFile("input", xComponentContext));
    CPPUNIT_ASSERT(xModel.is());

    Reference<awt::XUnoControlDialog> xDlg = awt::UnoControlDialog::create(xComponentContext);
    xDlg->setModel(xModel);
    xDlg->createPeer(xToolkit, 0);
    xDlg->execute();

    // write modified dialog
    exportToFile("output", xModel, xComponentContext);
}

CPPUNIT_TEST_SUITE_REGISTRATION(XmlScriptTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
