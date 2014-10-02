/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <vector>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <test/bootstrapfixture.hxx>
#include <vcl/svapp.hxx>

using namespace css::container;
using namespace css::reflection;
using namespace css::uno;

namespace {

class ServicesTest: public test::BootstrapFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(ServicesTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void ServicesTest::test()
{
    std::vector<OUString> blacklist;

    // On Windows, blacklist the com.sun.star.report.ReportDefinition service,
    // as its reportdesign::OReportDefinition implementation (in
    // reportdesign/source/core/api/ReportDefinition.cxx) spawns a thread that
    // forever blocks in SendMessageW when no VCL event loop is running
    // (reportdesign::<anon>::FactoryLoader::execute ->
    // framework::Desktop::findFrame -> framework::TaskCreator::createTask ->
    // <anon>::TaskCreatorService::createInstanceWithArguments ->
    // <anon>::TaskCreatorService::impls_createContainerWindow ->
    // <anon>::VCLXToolkit::createWindow ->
    // <anon>::VCLXToolkit::ImplCreateWindow ->
    // <anon>::VCLXToolkit::ImplCreateWindow -> WorkWindow::WorkWindow ->
    // WorkWindow::ImplInit -> ImplBorderWindow::ImplBorderWindow ->
    // ImplBorderWindow::ImplInit -> Window::ImplInit ->
    // WinSalInstance::CreateFrame -> ImplSendMessage -> SendMessageW):
    blacklist.push_back("com.sun.star.report.ReportDefinition");
    blacklist.push_back("com.sun.star.office.Quickstart");

    Reference< XHierarchicalNameAccess > xTypeManager(
            m_xContext->getValueByName(
                "/singletons/com.sun.star.reflection.theTypeDescriptionManager"),
            UNO_QUERY_THROW );
    Sequence<OUString> s = m_xContext->getServiceManager()->getAvailableServiceNames();
    std::vector< css::uno::Reference<css::lang::XComponent> > comps;
    for (sal_Int32 i = 0; i < s.getLength(); i++)
    {
        if (std::find(blacklist.begin(), blacklist.end(), s[i])
            != blacklist.end())
        {
            continue;
        }
        if (!xTypeManager->hasByHierarchicalName(s[i]))
        {
            SAL_WARN(
                "postprocess.cppunit",
                "fantasy service name \"" << s[i] << "\"");
            continue;
        }
        SAL_WARN(
                "postprocess.cppunit",
                "trying (index: " << i << ") \"" << s[i] << "\"");
        Reference< XServiceTypeDescription2 > xDesc(
            xTypeManager->getByHierarchicalName(s[i]), UNO_QUERY_THROW);
        Sequence< Reference< XServiceConstructorDescription > > xseq = xDesc->getConstructors();
        for (sal_Int32 c = 0; c < xseq.getLength(); c++)
            if (!xseq[c]->getParameters().hasElements())
            {
                Reference< XInterface > instance;
                try
                {
                    OString message = OUStringToOString(s[i], RTL_TEXTENCODING_UTF8);
                    bool bDefConstructor = xseq[c]->isDefaultConstructor();
                    Reference< css::lang::XMultiComponentFactory > serviceManager = m_xContext->getServiceManager();

                    if( bDefConstructor )
                        instance = serviceManager->createInstanceWithContext(s[i], m_xContext);
                    else
                        instance = serviceManager->createInstanceWithArgumentsAndContext(
                                                    s[i], css::uno::Sequence<css::uno::Any>(), m_xContext);

                    CPPUNIT_ASSERT_MESSAGE( message.getStr(), instance.is() );
                }
                catch(const Exception & e)
                {
                    OString exc = "Exception thrown while creating " +
                        OUStringToOString(s[i] + ": " + e.Message, RTL_TEXTENCODING_UTF8);
                    CPPUNIT_FAIL(exc.getStr());
                }
                css::uno::Reference<css::lang::XComponent> comp(
                    instance, css::uno::UNO_QUERY);
                if (comp.is()) {
                    comps.push_back(comp);
                }
            }
    }
    SolarMutexReleaser rel;
    for (std::vector< css::uno::Reference<css::lang::XComponent> >::iterator i(
             comps.begin());
         i != comps.end(); ++i)
    {
        (*i)->dispose();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ServicesTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
