/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Try to instantiate as many implementations as possible.  Finds all
// implementations reachable via the service manager.  If a given implementation
// is the only implementor of some service that has a zero-parameter
// constructor, instantiate the implementation through that service name.  If a
// given implementation does not offer any such constructors (because it does not
// support any single-interface--based service, or because for each relevant
// service there are multiple implementations or it does not have an appropriate
// constructor) but does support at least one accumulation-based service, then
// instantiate it through its implementation name (a heuristic to identify
// instantiatable implementations that appears to work well).

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/strbuf.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/svapp.hxx>

namespace {

OString msg(OUString const & string) {
    return OUStringToOString(string, osl_getThreadTextEncoding());
}

OString msg(css::uno::Sequence<OUString> const & strings) {
    OStringBuffer buf("{");
    for (sal_Int32 i = 0; i != strings.getLength(); ++i) {
        if (i != 0) {
            buf.append(", ");
        }
        buf.append('"');
        buf.append(msg(strings[i]));
        buf.append('"');
    }
    buf.append('}');
    return buf.makeStringAndClear();
}

bool unique(css::uno::Sequence<OUString> const & strings) {
    // Assumes small sequences for which quadratic algorithm is acceptable:
    for (sal_Int32 i = 0; i < strings.getLength() - 1; ++i) {
        for (sal_Int32 j = i + 1; j != strings.getLength(); ++j) {
            if (strings[j] == strings[i]) {
                return false;
            }
        }
    }
    return true;
}

bool contains(
    css::uno::Sequence<OUString> const & strings, OUString const & string)
{
    for (sal_Int32 i = 0; i != strings.getLength(); ++i) {
        if (string == strings[i]) {
            return true;
        }
    }
    return false;
}

bool contains(
    css::uno::Sequence<OUString> const & strings1,
    css::uno::Sequence<OUString> const & strings2)
{
    // Assumes small sequences for which quadratic algorithm is acceptable:
    for (sal_Int32 i = 0; i != strings2.getLength(); ++i) {
        if (!contains(strings1, strings2[i])) {
            return false;
        }
    }
    return true;
}

class Test: public test::BootstrapFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    void createInstance(
        OUString const & name, bool withArguments,
        OUString const & implementationName,
        css::uno::Sequence<OUString> const & serviceNames,
        std::vector<css::uno::Reference<css::lang::XComponent>> * components);
};

void Test::test() {
    // On Windows, blacklist the com.sun.star.comp.report.OReportDefinition
    // implementation (reportdesign::OReportDefinition in
    // reportdesign/source/core/api/ReportDefinition.cxx), as it spawns a thread
    // that forever blocks in SendMessageW when no VCL event loop is running
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
    std::vector<OUString> blacklist;
    blacklist.emplace_back("com.sun.star.comp.report.OReportDefinition");

    // <https://bugs.documentfoundation.org/show_bug.cgi?id=89343>
    // "~SwXMailMerge() goes into endless SwCache::Check()":
    blacklist.emplace_back("SwXMailMerge");

    css::uno::Reference<css::container::XContentEnumerationAccess> enumAcc(
        m_xContext->getServiceManager(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::container::XHierarchicalNameAccess> typeMgr(
        m_xContext->getValueByName(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence<OUString> serviceNames(
        m_xContext->getServiceManager()->getAvailableServiceNames());
    struct Constructor {
        Constructor(
            OUString const & theServiceName, bool theDefaultConstructor):
            serviceName(theServiceName),
            defaultConstructor(theDefaultConstructor)
        {}
        OUString serviceName;
        bool defaultConstructor;
    };
    struct Implementation {
        Implementation(
            css::uno::Reference<css::lang::XServiceInfo> const & theFactory,
            css::uno::Sequence<OUString> const & theServiceNames):
            factory(theFactory), serviceNames(theServiceNames),
            accumulationBased(false)
        {}
        css::uno::Reference<css::lang::XServiceInfo> const factory;
        css::uno::Sequence<OUString> const serviceNames;
        std::vector<Constructor> constructors;
        bool accumulationBased;
    };
    std::map<OUString, Implementation> impls;
    for (sal_Int32 i = 0; i != serviceNames.getLength(); ++i) {
        css::uno::Reference<css::container::XEnumeration> serviceImpls1(
            enumAcc->createContentEnumeration(serviceNames[i]),
            css::uno::UNO_SET_THROW);
        std::vector<css::uno::Reference<css::lang::XServiceInfo>> serviceImpls2;
        while (serviceImpls1->hasMoreElements()) {
            serviceImpls2.emplace_back(
                    serviceImpls1->nextElement(), css::uno::UNO_QUERY_THROW);
        }
        css::uno::Reference<css::reflection::XServiceTypeDescription2> desc;
        if (typeMgr->hasByHierarchicalName(serviceNames[i])) {
            desc.set(
                typeMgr->getByHierarchicalName(serviceNames[i]),
                css::uno::UNO_QUERY_THROW);
        }
        if (serviceImpls2.empty()) {
            if (desc.is()) {
                CPPUNIT_ASSERT_MESSAGE(
                    (OString(
                        "no implementations of single-interface--based \""
                        + msg(serviceNames[i]) + "\"")
                     .getStr()),
                    !desc->isSingleInterfaceBased());
                std::cout
                    << "accumulation-based service \"" << serviceNames[i]
                    << "\" without implementations\n";
            } else {
                std::cout
                    << "fantasy service name \"" << serviceNames[i]
                    << "\" without implementations\n";
            }
        } else {
            for (auto const & j: serviceImpls2) {
                OUString name(j->getImplementationName());
                auto k = impls.find(name);
                if (k == impls.end()) {
                    css::uno::Sequence<OUString> servs(
                        j->getSupportedServiceNames());
                    CPPUNIT_ASSERT_MESSAGE(
                        (OString(
                            "implementation \"" + msg(name)
                            + "\" supports non-unique " + msg(servs))
                         .getStr()),
                        unique(servs));
                    k = impls.insert(
                            std::make_pair(name, Implementation(j, servs)))
                        .first;
                } else {
                    CPPUNIT_ASSERT_MESSAGE(
                        (OString(
                            "multiple implementations named \"" + msg(name)
                            + "\"")
                         .getStr()),
                        bool(j == k->second.factory));
                }
                CPPUNIT_ASSERT_MESSAGE(
                    (OString(
                        "implementation \"" + msg(name) + "\" supports "
                        + msg(k->second.serviceNames) + " but not \""
                        + msg(serviceNames[i]) + "\"")
                     .getStr()),
                    contains(k->second.serviceNames, serviceNames[i]));
                if (desc.is()) {
                    if (desc->isSingleInterfaceBased()) {
                        if (serviceImpls2.size() == 1) {
                            css::uno::Sequence<
                                css::uno::Reference<
                                    css::reflection::XServiceConstructorDescription>>
                                        ctors(desc->getConstructors());
                            for (sal_Int32 l = 0; l != ctors.getLength(); ++l) {
                                if (!ctors[l]->getParameters().hasElements()) {
                                    k->second.constructors.emplace_back(
                                            serviceNames[i],
                                            ctors[l]->isDefaultConstructor());
                                    break;
                                }
                            }
                        }
                    } else {
                        k->second.accumulationBased = true;
                    }
                } else {
                    std::cout
                        << "implementation \"" << name
                        << "\" supports fantasy service name \""
                        << serviceNames[i] << "\"\n";
                }
            }
        }
    }
    std::vector<css::uno::Reference<css::lang::XComponent>> comps;
    for (auto const & i: impls) {
        if (std::find(blacklist.begin(), blacklist.end(), i.first)
            == blacklist.end())
        {
            if (i.second.constructors.empty()) {
                if (i.second.accumulationBased) {
                    createInstance(
                        i.first, false, i.first, i.second.serviceNames, &comps);
                } else {
                    std::cout
                        << "no obvious way to instantiate implementation \""
                        << i.first << "\"\n";
                }
            } else {
                for (auto const & j: i.second.constructors) {
                    createInstance(
                        j.serviceName, !j.defaultConstructor, i.first,
                        i.second.serviceNames, &comps);
                }
            }
        }
    }
    SolarMutexReleaser rel;
    for (auto const & i: comps) {
        i->dispose();
    }
}

void Test::createInstance(
    OUString const & name, bool withArguments,
    OUString const & implementationName,
    css::uno::Sequence<OUString> const & serviceNames,
    std::vector<css::uno::Reference<css::lang::XComponent>> * components)
{
    assert(components != nullptr);
    css::uno::Reference<css::uno::XInterface> inst;
    try {
        if (withArguments) {
            inst = m_xContext->getServiceManager()
                ->createInstanceWithArgumentsAndContext(
                    name, css::uno::Sequence<css::uno::Any>(), m_xContext);
        } else {
            inst = m_xContext->getServiceManager()->createInstanceWithContext(
                name, m_xContext);
        }
    } catch (css::uno::Exception & e) {
        css::uno::Any a(cppu::getCaughtException());
        CPPUNIT_FAIL(
            OString(
                "instantiating \"" + msg(implementationName) + "\" via \""
                + msg(name) + "\"  caused " + msg(a.getValueTypeName()) + " \""
                + msg(e.Message) + "\"")
            .getStr());
    }
    CPPUNIT_ASSERT_MESSAGE(
        (OString(
            "instantiating \"" + msg(implementationName) + "\" via \""
            + msg(name) + "\" returned null reference")
         .getStr()),
        inst.is());
    css::uno::Reference<css::lang::XComponent> comp(inst, css::uno::UNO_QUERY);
    if (comp.is()) {
        components->push_back(comp);
    }
    css::uno::Reference<css::lang::XServiceInfo> info(
        inst, css::uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE(
        (OString(
            "instantiating \"" + msg(implementationName) + "\" via \""
            + msg(name) + "\" does not provide XServiceInfo")
         .getStr()),
        info.is());
    OUString expImpl(implementationName);
    css::uno::Sequence<OUString> expServs(serviceNames);
    // Special cases:
    if (name == "com.sun.star.comp.configuration.ConfigurationProvider") {
        // Instantiating a ConfigurationProvider with no or empty args must
        // return theDefaultProvider:
        expImpl = "com.sun.star.comp.configuration.DefaultProvider";
        expServs = {"com.sun.star.configuration.DefaultProvider"};
    } else if (name == "com.sun.star.datatransfer.clipboard.SystemClipboard") {
        // SystemClipboard is a wrapper returning either a platform-specific or
        // the generic VCLGenericClipboard:
#if defined(_WIN32)
        expImpl = "com.sun.star.datatransfer.clipboard.ClipboardW32";
#else
        expImpl = "com.sun.star.datatransfer.VCLGenericClipboard";
#endif
#if !defined(_WIN32)
    } else if (name == "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"
               || name == "com.sun.star.datatransfer.dnd.XdndSupport")
    {
        expImpl = "com.sun.star.datatransfer.dnd.VclGenericDragSource";
        expServs = {"com.sun.star.datatransfer.dnd.GenericDragSource"};
    } else if (name == "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"
               || name == "com.sun.star.datatransfer.dnd.XdndDropTarget")
    {
        expImpl = "com.sun.star.datatransfer.dnd.VclGenericDropTarget";
        expServs = {"com.sun.star.datatransfer.dnd.GenericDropTarget"};
#endif
    } else if (name == "com.sun.star.ui.dialogs.FolderPicker") {
        // FolderPicker is a wrapper returning either a platform-specific or the
        // generic OfficeFolderPicker:
#if defined(_WIN32)
        expImpl = "com.sun.star.ui.dialogs.Win32FolderPicker";
        expServs = {"com.sun.star.ui.dialogs.SystemFolderPicker"};
#else
        expImpl = "com.sun.star.svtools.OfficeFolderPicker";
        expServs = {"com.sun.star.ui.dialogs.OfficeFolderPicker"};
#endif
    } else if (expImpl == "com.sun.star.comp.Calc.SpreadsheetDocument") {
        expImpl = "ScModelObj";
    } else if (expImpl == "com.sun.star.comp.Draw.DrawingDocument"
               || expImpl == "com.sun.star.comp.Draw.PresentationDocument")
    {
        expImpl = "SdXImpressDocument";
    } else if (expImpl == "com.sun.star.comp.Writer.GlobalDocument"
               || expImpl == "com.sun.star.comp.Writer.TextDocument"
               || expImpl == "com.sun.star.comp.Writer.WebDocument")
    {
        expImpl = "SwXTextDocument";
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        (OString(
            "instantiating \"" + msg(implementationName) + "\" via \""
            + msg(name) + "\" reports wrong implementation name")
         .getStr()),
        expImpl, info->getImplementationName());
    css::uno::Sequence<OUString> servs(info->getSupportedServiceNames());
    CPPUNIT_ASSERT_MESSAGE(
        (OString(
            "instantiating \"" + msg(implementationName) + "\" via \""
            + msg(name) + "\" reports non-unique " + msg(servs))
         .getStr()),
        unique(servs));
    // Some implementations like "com.sun.star.comp.Calc.SpreadsheetDocument"
    // report sub-services like
    // "com.sun.star.sheet.SpreadsheetDocumentSettings", and
    // "com.sun.star.document.OfficeDocument" that are not listed in the
    // .component file, so check for containment instead of equality:
    CPPUNIT_ASSERT_MESSAGE(
        (OString(
            "instantiating \"" + msg(implementationName) + "\" via \""
            + msg(name) + "\" reports " + msg(servs) + " different from "
            + msg(expServs))
         .getStr()),
        contains(servs, expServs));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
