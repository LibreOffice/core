/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_QA_UNIT_WPFTFILTERTESTBASE_HXX
#define INCLUDED_WRITERPERFECT_QA_UNIT_WPFTFILTERTESTBASE_HXX

#include "wpftqahelperdllapi.h"

#include <unordered_map>

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustring.hxx>

#include <unotest/filters-test.hxx>

#include "WpftFilterFixture.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace document
{
class XFilter;
}
}
}
}

namespace writerperfect
{
namespace test
{

typedef std::unordered_map<rtl::OUString, bool, rtl::OUStringHash> WpftOptionalMap_t;

class WPFTQAHELPER_DLLPUBLIC WpftFilterTestBase
    : public ::test::FiltersTest
    , public WpftFilterFixture
{
public:
    explicit WpftFilterTestBase(const rtl::OUString &rFactoryURL);

protected:
    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath);
    void doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath, const WpftOptionalMap_t &rOptionalMap);

private:
    virtual bool load(const OUString &, const OUString &rURL, const OUString &,
                      SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

private:
    const rtl::OUString m_aFactoryURL;
    css::uno::Reference<css::document::XFilter> m_xFilter;
    const WpftOptionalMap_t *m_pOptionalMap;
};

}
}

#endif // INCLUDED_WRITERPERFECT_QA_UNIT_WPFTFILTERTESTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
