/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hdl>
#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <forms/gridcolumnproptranslator.hxx>
#include <comphelper/propertysethelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace
{
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/xmloff/qa/unit/data/"_ustr)
    {
    }
};

class MockColumnAlignmentPropertySetInfo : public XPropertySetInfo
{
public:
    // XPropertySetInfo
    Sequence<Property> SAL_CALL getProperties() override
    {
        return { getPropertyByName(u"Align"_ustr) };
    }

    Property SAL_CALL getPropertyByName(const OUString& aName) override
    {
        return Property{ aName, /*handle*/ 0, /*type*/ cppu::UnoType<sal_Int16>::get(),
                         /*attributes*/ 0 };
    }

    sal_Bool SAL_CALL hasPropertyByName(const OUString& aName) override
    {
        return aName == u"Align"_ustr;
    }

    // XInterface
    Any SAL_CALL queryInterface(const Type& aType) override
    {
        Any aRet;
        if (aType == cppu::UnoType<XPropertySetInfo>::get())
        {
            aRet <<= Reference<XPropertySetInfo>(this);
        }

        return aRet;
    }

    void SAL_CALL acquire() override {}

    void SAL_CALL release() override {}
};

class MockColumnAlignmentMultiPropertySet : public XMultiPropertySet
{
private:
    sal_Int16 nCurrTextAlign = awt::TextAlign::LEFT;

public:
    // XMultiPropertySet
    Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return { new MockColumnAlignmentPropertySetInfo };
    }

    void SAL_CALL setPropertyValues(const Sequence<OUString>& aPropertyNames,
                                    const Sequence<Any>& aValues) override
    {
        for (sal_Int32 nIndex = 0; nIndex < aPropertyNames.getLength(); ++nIndex)
        {
            if (aPropertyNames[nIndex] == u"Align"_ustr)
            {
                aValues[nIndex] >>= nCurrTextAlign;
            }
        }
    }

    Sequence<Any> SAL_CALL getPropertyValues(const Sequence<OUString>& aPropertyNames) override
    {
        Sequence<Any> aRet{ aPropertyNames.getLength() };
        Any* pOut = aRet.getArray();

        for (sal_Int32 nIndex = 0; nIndex < aPropertyNames.getLength(); ++nIndex)
        {
            if (aPropertyNames[nIndex] == u"Align"_ustr)
            {
                *pOut <<= nCurrTextAlign;
            }

            ++pOut;
        }

        return aRet;
    }

    void SAL_CALL
    addPropertiesChangeListener(const Sequence<OUString>& /*aPropertyNames*/,
                                const Reference<XPropertiesChangeListener>& /*xListener*/) override
    {
    }

    void SAL_CALL removePropertiesChangeListener(
        const Reference<XPropertiesChangeListener>& /*xListener*/) override
    {
    }

    void SAL_CALL
    firePropertiesChangeEvent(const Sequence<OUString>& /*aPropertyNames*/,
                              const Reference<XPropertiesChangeListener>& /*xListener*/) override
    {
    }

    // XInterface
    Any SAL_CALL queryInterface(const Type& aType) override
    {
        Any aRet;
        if (aType == cppu::UnoType<XMultiPropertySet>::get())
        {
            aRet <<= Reference<XMultiPropertySet>(this);
        }

        return aRet;
    }

    void SAL_CALL acquire() override {}

    void SAL_CALL release() override {}
};
}

CPPUNIT_TEST_FIXTURE(Test, testColumnAlignmentConversion)
{
    uno::Reference<XMultiPropertySet> xProps = new MockColumnAlignmentMultiPropertySet;
    CPPUNIT_ASSERT(xProps.is());
    Reference<XPropertySet> xTranslator = new xmloff::OGridColumnPropertyTranslator{ xProps };
    CPPUNIT_ASSERT(xTranslator.is());

    auto fnSetParaAdjust = [&](style::ParagraphAdjust nValue) {
        // Intentionally set the property as sal_Int32, not ParagraphAdjust.
        // OGridColumnPropertyTranslator cannot translate the enum value.
        xTranslator->setPropertyValue(u"ParaAdjust"_ustr, Any{ static_cast<sal_Int32>(nValue) });
    };

    auto fnGetParaAdjust = [&] {
        // However, get the property as ParagraphAdjust, because this is
        // what OGridColumnPropertyTranslator returns.
        Any aSource = xTranslator->getPropertyValue(u"ParaAdjust"_ustr);

        style::ParagraphAdjust nValue = style::ParagraphAdjust_LEFT;
        aSource >>= nValue;

        return nValue;
    };

    auto fnSetTextAlign
        = [&](sal_Int16 nValue) { xTranslator->setPropertyValue(u"Align"_ustr, Any{ nValue }); };

    auto fnGetTextAlign = [&] {
        Any aSource = xTranslator->getPropertyValue(u"Align"_ustr);

        sal_Int32 nValue = 0;
        aSource >>= nValue;

        return nValue;
    };

    // Sanity check: should be initialized to left adjusted
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, fnGetParaAdjust());
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::LEFT, fnGetTextAlign());

    // Sanity check: paragraph adjust and AWT text align should round-trip
    fnSetTextAlign(awt::TextAlign::RIGHT);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::RIGHT, fnGetTextAlign());

    fnSetTextAlign(awt::TextAlign::LEFT);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::LEFT, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_RIGHT);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_RIGHT, fnGetParaAdjust());

    fnSetParaAdjust(style::ParagraphAdjust_LEFT);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, fnGetParaAdjust());

    // Check conversion from AWT text align to paragraph adjust
    fnSetTextAlign(awt::TextAlign::LEFT);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, fnGetParaAdjust());

    fnSetTextAlign(awt::TextAlign::CENTER);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, fnGetParaAdjust());

    fnSetTextAlign(awt::TextAlign::RIGHT);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_RIGHT, fnGetParaAdjust());

    // Check conversion from paragraph adjust to AWT text align
    fnSetParaAdjust(style::ParagraphAdjust_LEFT);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::LEFT, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_CENTER);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::CENTER, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_RIGHT);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::RIGHT, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_BLOCK);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::RIGHT, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_STRETCH);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::LEFT, fnGetTextAlign());

    // Like Calc, Base treats left/right align as start/end. RTL support
    // is provided through RTL UI, which mirrors paragraph direction and
    // alignment. Just check that these values are remapped correctly.
    fnSetParaAdjust(style::ParagraphAdjust_START);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::LEFT, fnGetTextAlign());

    fnSetParaAdjust(style::ParagraphAdjust_END);
    CPPUNIT_ASSERT_EQUAL(awt::TextAlign::RIGHT, fnGetTextAlign());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
