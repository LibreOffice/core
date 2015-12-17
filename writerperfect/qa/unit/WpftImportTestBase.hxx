/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_QA_UNIT_WPFTIMPORTTESTBASE_HXX
#define INCLUDED_WRITERPERFECT_QA_UNIT_WPFTIMPORTTESTBASE_HXX

#include "config_writerperfect.h"

#include <unordered_map>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>

#include <test/bootstrapfixture.hxx>

#include <unotest/filters-test.hxx>

#define REQUIRE_VERSION(major, minor, micro, req_major, req_minor, req_micro) \
    (major) > (req_major) || \
    ((major) == (req_major) && \
        ((minor) > (req_minor) \
         || ((minor) == (req_minor) && ((micro) >= (req_micro)))))

#define REQUIRE_ETONYEK_VERSION(major, minor, micro) \
    REQUIRE_VERSION(ETONYEK_VERSION_MAJOR, ETONYEK_VERSION_MINOR, ETONYEK_VERSION_MICRO, major, minor, micro)

#define REQUIRE_MWAW_VERSION(major, minor, micro) \
    REQUIRE_VERSION(MWAW_VERSION_MAJOR, MWAW_VERSION_MINOR, MWAW_VERSION_MICRO, major, minor, micro)

namespace com
{
namespace sun
{
namespace star
{
namespace beans
{
struct PropertyValue;
}
namespace container
{
class XNameAccess;
}
namespace document
{
class XFilter;
}
namespace frame
{
class XDesktop2;
}
namespace ucb
{
class XSimpleFileAccess;
}
}
}
}

namespace writerperfect
{
namespace test
{

typedef std::unordered_map<rtl::OUString, bool, rtl::OUStringHash> WpftOptionalMap_t;

class WpftImportTestBase
    : public ::test::FiltersTest
    , public ::test::BootstrapFixture
{
public:
    explicit WpftImportTestBase(const rtl::OUString &rFactoryURL);

    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath);
    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath, const WpftOptionalMap_t &rOptionalMap);

private:
    virtual bool load(const OUString &, const OUString &rURL, const OUString &,
                      SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    void impl_detectFilterName(css::uno::Sequence<css::beans::PropertyValue> &rDescriptor, const rtl::OUString &rTypeName);

private:
    const rtl::OUString m_aFactoryURL;
    css::uno::Reference<css::frame::XDesktop2> m_xDesktop;
    css::uno::Reference<css::ucb::XSimpleFileAccess> m_xFileAccess;
    css::uno::Reference<css::document::XFilter> m_xFilter;
    css::uno::Reference<css::container::XNameAccess> m_xTypeMap;
    const WpftOptionalMap_t *m_pOptionalMap;
};

}
}

#endif // INCLUDED_WRITERPERFECT_QA_UNIT_WPFTIMPORTTESTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
