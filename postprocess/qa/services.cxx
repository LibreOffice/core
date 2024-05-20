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
// instantiable implementations that appears to work well).

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/strbuf.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/svapp.hxx>

namespace {

OString msg(std::u16string_view string) {
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
    return comphelper::findValue(strings, string) != -1;
}

bool contains(
    css::uno::Sequence<OUString> const & strings1,
    css::uno::Sequence<OUString> const & strings2)
{
    // Assumes small sequences for which quadratic algorithm is acceptable:
    return std::all_of(strings2.begin(), strings2.end(),
        [&strings1](const OUString& rStr) { return contains(strings1, rStr); });
}

void addService(
    css::uno::Reference<css::reflection::XServiceTypeDescription> const & service,
    std::set<css::uno::Reference<css::reflection::XServiceTypeDescription>> * allServices)
{
    assert(allServices != nullptr);
    if (!allServices->insert(service).second) {
        return;
    }
    const auto aMandatoryServices = service->getMandatoryServices();
    for (auto const & serv : aMandatoryServices) {
        addService(serv, allServices);
    }
}

class Test: public test::BootstrapFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    void createInstance(
        css::uno::Reference<css::container::XHierarchicalNameAccess> const & typeManager,
        OUString const & name, bool withArguments,
        OUString const & implementationName,
        css::uno::Sequence<OUString> const & serviceNames,
        std::vector<css::uno::Reference<css::lang::XComponent>> * components);
};

void Test::test() {
    // On Windows, denylist the com.sun.star.comp.report.OReportDefinition
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
    std::vector<OUString> denylist;
    denylist.emplace_back("com.sun.star.comp.report.OReportDefinition");

    // <https://bugs.documentfoundation.org/show_bug.cgi?id=89343>
    // "~SwXMailMerge() goes into endless SwCache::Check()":
    denylist.emplace_back("SwXMailMerge");

    css::uno::Reference<css::container::XContentEnumerationAccess> enumAcc(
        m_xContext->getServiceManager(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::container::XHierarchicalNameAccess> typeMgr(
        m_xContext->getValueByName(
            u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
        css::uno::UNO_QUERY_THROW);
    const css::uno::Sequence<OUString> serviceNames(
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
    for (const auto& rServiceName : serviceNames) {
        css::uno::Reference<css::container::XEnumeration> serviceImpls1(
            enumAcc->createContentEnumeration(rServiceName),
            css::uno::UNO_SET_THROW);
        std::vector<css::uno::Reference<css::lang::XServiceInfo>> serviceImpls2;
        while (serviceImpls1->hasMoreElements()) {
            serviceImpls2.emplace_back(
                    serviceImpls1->nextElement(), css::uno::UNO_QUERY_THROW);
        }
        css::uno::Reference<css::reflection::XServiceTypeDescription2> desc;
        if (typeMgr->hasByHierarchicalName(rServiceName)) {
            desc.set(
                typeMgr->getByHierarchicalName(rServiceName),
                css::uno::UNO_QUERY_THROW);
        }
        if (serviceImpls2.empty()) {
            if (desc.is()) {
                CPPUNIT_ASSERT_MESSAGE(
                    (OString(
                        "no implementations of single-interface--based \""
                        + msg(rServiceName) + "\"")
                     .getStr()),
                    !desc->isSingleInterfaceBased());
                std::cout
                    << "accumulation-based service \"" << rServiceName
                    << "\" without implementations\n";
            } else {
                std::cout
                    << "fantasy service name \"" << rServiceName
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
                        + msg(rServiceName) + "\"")
                     .getStr()),
                    contains(k->second.serviceNames, rServiceName));
                if (desc.is()) {
                    if (desc->isSingleInterfaceBased()) {
                        if (serviceImpls2.size() == 1) {
                            const css::uno::Sequence<
                                css::uno::Reference<
                                    css::reflection::XServiceConstructorDescription>>
                                        ctors(desc->getConstructors());
                            auto pCtor = std::find_if(ctors.begin(), ctors.end(),
                                [](const auto& rCtor) { return !rCtor->getParameters().hasElements(); });
                            if (pCtor != ctors.end())
                                k->second.constructors.emplace_back(
                                        rServiceName,
                                        (*pCtor)->isDefaultConstructor());
                        }
                    } else {
                        k->second.accumulationBased = true;
                    }
                } else {
                    std::cout
                        << "implementation \"" << name
                        << "\" supports fantasy service name \""
                        << rServiceName << "\"\n";
                }
            }
        }
    }
    std::vector<css::uno::Reference<css::lang::XComponent>> comps;
    for (auto const & i: impls) {
        if (std::find(denylist.begin(), denylist.end(), i.first)
            == denylist.end())
        {
            if (i.second.constructors.empty()) {
                if (i.second.accumulationBased) {
                    createInstance(
                        typeMgr, i.first, false, i.first, i.second.serviceNames, &comps);
                } else {
                    std::cout
                        << "no obvious way to instantiate implementation \""
                        << i.first << "\"\n";
                }
            } else {
                for (auto const & j: i.second.constructors) {
                    createInstance(
                        typeMgr, j.serviceName, !j.defaultConstructor, i.first,
                        i.second.serviceNames, &comps);
                }
            }
        }
    }
    SolarMutexReleaser rel;
    for (auto const & i: comps) {
        // cannot call dispose() on XDesktop before calling terminate()
        if (!css::uno::Reference<css::frame::XDesktop>(i, css::uno::UNO_QUERY))
            i->dispose();
    }
}

void Test::createInstance(
    css::uno::Reference<css::container::XHierarchicalNameAccess> const & typeManager,
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
        expServs = {u"com.sun.star.configuration.DefaultProvider"_ustr};
    } else if (name == "com.sun.star.datatransfer.clipboard.SystemClipboard") {
        // SystemClipboard is a wrapper returning either a platform-specific or
        // the generic VCLGenericClipboard:
        expImpl = "com.sun.star.datatransfer.VCLGenericClipboard";
#if !defined(_WIN32)
    } else if (name == "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"
               || name == "com.sun.star.datatransfer.dnd.XdndSupport")
    {
        expImpl = "com.sun.star.datatransfer.dnd.VclGenericDragSource";
        expServs = {u"com.sun.star.datatransfer.dnd.GenericDragSource"_ustr};
    } else if (name == "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"
               || name == "com.sun.star.datatransfer.dnd.XdndDropTarget")
    {
        expImpl = "com.sun.star.datatransfer.dnd.VclGenericDropTarget";
        expServs = {u"com.sun.star.datatransfer.dnd.GenericDropTarget"_ustr};
#endif
    } else if (name == "com.sun.star.ui.dialogs.FolderPicker") {
        // FolderPicker is a wrapper returning either a platform-specific or the
        // generic OfficeFolderPicker. In headless mode it is always the
        // generic one.
        expImpl = "com.sun.star.svtools.OfficeFolderPicker";
        expServs = {u"com.sun.star.ui.dialogs.OfficeFolderPicker"_ustr};
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
    const css::uno::Sequence<OUString> servs(info->getSupportedServiceNames());
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
    std::set<css::uno::Reference<css::reflection::XServiceTypeDescription>> allservs;
    for (auto const & serv: servs) {
        if (!typeManager->hasByHierarchicalName(serv)) {
            std::cout
                << "instantiating \"" << implementationName << "\" via \"" << name
                << "\" supports fantasy service name \"" << serv << "\"\n";
            continue;
        }
        addService(
            css::uno::Reference<css::reflection::XServiceTypeDescription>(
                typeManager->getByHierarchicalName(serv), css::uno::UNO_QUERY_THROW),
            &allservs);
    }
    css::uno::Reference<css::beans::XPropertySetInfo> propsinfo;
    for (auto const & serv: allservs) {
        auto const props = serv->getProperties();
        for (auto const & prop: props) {
            auto const optional
                = (prop->getPropertyFlags() & css::beans::PropertyAttribute::OPTIONAL) != 0;
            if (!propsinfo.is()) {
                css::uno::Reference<css::beans::XPropertySet> propset(inst, css::uno::UNO_QUERY);
                if (!propset.is()) {
                    CPPUNIT_ASSERT_MESSAGE(
                        (OString(
                            "instantiating \"" + msg(implementationName) + "\" via \"" + msg(name)
                            + "\" reports service " + msg(serv->getName())
                            + " with non-optional property \"" + msg(prop->getName())
                            + "\" but does not implement css.uno.XPropertySet")
                         .getStr()),
                        optional);
                    continue;
                }
                propsinfo = propset->getPropertySetInfo();
                if (!propsinfo.is()) {
                    //TODO: legal to return null in more cases? ("@returns NULL if the
                    // implementation cannot or will not provide information about the properties")
                    CPPUNIT_ASSERT_MESSAGE(
                        (OString(
                            "instantiating \"" + msg(implementationName) + "\" via \"" + msg(name)
                            + "\" reports service " + msg(serv->getName())
                            + " with non-optional property \"" + msg(prop->getName())
                            + "\" but css.uno.XPropertySet::getPropertySetInfo returns null")
                         .getStr()),
                        optional);
                    continue;
                }
            }
            if (!propsinfo->hasPropertyByName(prop->getName())) {
                static std::set<std::pair<OUString, OUString>> const denylist{
                    {"com.sun.star.comp.chart.DataSeries", "BorderDash"},
                    {"com.sun.star.comp.chart2.ChartDocumentWrapper", "UserDefinedAttributes"},
                    {"com.sun.star.comp.dbu.OColumnControlModel", "Tabstop"},
                    {"com.sun.star.comp.report.OFormattedField", "Align"},
                    {"com.sun.star.comp.report.OFormattedField", "BackgroundColor"},
                    {"com.sun.star.comp.report.OFormattedField", "Border"},
                    {"com.sun.star.comp.report.OFormattedField", "DefaultControl"},
                    {"com.sun.star.comp.report.OFormattedField", "EffectiveDefault"},
                    {"com.sun.star.comp.report.OFormattedField", "EffectiveMax"},
                    {"com.sun.star.comp.report.OFormattedField", "EffectiveMin"},
                    {"com.sun.star.comp.report.OFormattedField", "EffectiveValue"},
                    {"com.sun.star.comp.report.OFormattedField", "Enabled"},
                    {"com.sun.star.comp.report.OFormattedField", "FontEmphasisMark"},
                    {"com.sun.star.comp.report.OFormattedField", "FontRelief"},
                    {"com.sun.star.comp.report.OFormattedField", "HelpText"},
                    {"com.sun.star.comp.report.OFormattedField", "HelpURL"},
                    {"com.sun.star.comp.report.OFormattedField", "MaxTextLen"},
                    {"com.sun.star.comp.report.OFormattedField", "Printable"},
                    {"com.sun.star.comp.report.OFormattedField", "ReadOnly"},
                    {"com.sun.star.comp.report.OFormattedField", "Spin"},
                    {"com.sun.star.comp.report.OFormattedField", "Tabstop"},
                    {"com.sun.star.comp.report.OFormattedField", "Text"},
                    {"com.sun.star.comp.report.OFormattedField", "TextColor"},
                    {"com.sun.star.comp.report.OFormattedField", "TextLineColor"},
                    {"com.sun.star.comp.report.OFormattedField", "TreatAsNumber"},
                    {"stardiv.Toolkit.UnoControlRoadmapModel", "Interactive"}};
                if (denylist.find({implementationName, prop->getName()}) != denylist.end()) {
                    continue;
                }
                CPPUNIT_ASSERT_MESSAGE(
                    (OString(
                        "instantiating \"" + msg(implementationName) + "\" via \"" + msg(name)
                        + "\" reports service " + msg(serv->getName())
                        + " with non-optional property \"" + msg(prop->getName())
                        + ("\" but css.uno.XPropertySet::getPropertySetInfo's hasPropertyByName"
                           " returns false"))
                     .getStr()),
                    optional);
            }
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
