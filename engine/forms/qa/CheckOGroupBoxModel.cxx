/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cpo/uno/TypeClass.hpp>

using namespace ::css::awt;
using namespace ::css::beans;
using namespace ::css::drawing;
using namespace ::css::lang;
using namespace ::cpo::uno;

namespace
{
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/forms/qa/cppunit/data/"_ustr)
    {
    }
};

/**
 * creates a XControlShape
 *
 * @param oDoc the document
 * @param height the height of the shape
 * @param width the width of the shape
 * @param x the x-position of the shape
 * @param y the y-position of the shape
 * @param kind the kind of the shape
 * @return the created XControlShape
*/
Reference<XControlShape> createControlShape(const Reference<XComponent>& oDoc, sal_Int32 height,
                                            sal_Int32 width, sal_Int32 x, sal_Int32 y,
                                            std::u16string_view kind)
{
    Reference<XMultiServiceFactory> oDocMSF(oDoc, UNO_QUERY);

    Reference<XInterface> oInt = oDocMSF->createInstance(u"com.sun.star.drawing.ControlShape"_ustr);
    Reference<XInterface> aCon
        = oDocMSF->createInstance(OUString::Concat("com.sun.star.form.component.") + kind);
    Reference<XPropertySet> model_props(aCon, UNO_QUERY);
    model_props->setPropertyValue(u"DefaultControl"_ustr,
                                  Any(OUString::Concat("com.sun.star.form.control.") + kind));
    Reference<XControlModel> aControl(aCon, UNO_QUERY);
    Reference<XControlShape> oCShape(oInt, UNO_QUERY);
    css::awt::Size size;
    size.Height = height;
    size.Width = width;
    css::awt::Point position;
    position.X = x;
    position.Y = y;
    oCShape->setSize(size);
    oCShape->setPosition(position);

    oCShape->setControl(aControl);

    return oCShape;
}

/**
 * gets the specified XDrawPage of a draw document
 *
 * @param aDoc the draw document
 * @param nr the index of the DrawPage
 * @return the XDrawpage with index nr of the document
*/
Reference<XDrawPage> getDrawPage(const Reference<XComponent>& aDoc, sal_Int32 nr)
{
    Reference<XDrawPagesSupplier> oDPS(aDoc, UNO_QUERY);
    Reference<XDrawPage> oDP;
    oDPS->getDrawPages()->getByIndex(nr) >>= oDP;
    return oDP;
}

Reference<XControlShape> insertControlShape(const Reference<XComponent>& oDoc, sal_Int32 height,
                                            sal_Int32 width, sal_Int32 x, sal_Int32 y,
                                            std::u16string_view kind)
{
    Reference<XControlShape> aShape = createControlShape(oDoc, height, width, x, y, kind);
    Reference<XDrawPage> oDP = getDrawPage(oDoc, 0);
    oDP->add(aShape);
    return aShape;
}

Sequence<OUString> getBoundPropsToTest(const Reference<XMultiPropertySet>& xPropSet)
{
    Sequence<Property> properties = xPropSet->getPropertySetInfo()->getProperties();
    std::vector<OUString> tNames;

    for (const Property& property : properties)
    {
        bool isWritable = ((property.Attributes & PropertyAttribute::READONLY) == 0);
        bool isNotNull = ((property.Attributes & PropertyAttribute::MAYBEVOID) == 0);
        bool isBound = ((property.Attributes & PropertyAttribute::BOUND) != 0);

        if (isWritable && isNotNull && isBound)
        {
            tNames.push_back(property.Name);
        }
    }

    return comphelper::containerToSequence(tNames);
}

/**
 * Listener implementation which sets a flag when
 * listener was called.
 */
class MyChangeListener : public cppu::WeakImplHelper<XPropertiesChangeListener>
{
    bool propertiesChanged = false;

public:
    void propertiesChange(const Sequence<PropertyChangeEvent>&) override
    {
        propertiesChanged = true;
    }

    void disposing(const EventObject&) override {}

    bool wasListenerCalled() { return propertiesChanged; }
};

// Method to change a Value, thought for properties
Any changePValue(const Any& oldValue)
{
    Any newValue;

    switch (oldValue.getValueTypeClass())
    {
        case TypeClass_BOOLEAN:
        {
            auto oldbool = oldValue.get<bool>();
            newValue <<= !oldbool;
            return newValue;
        }
        case TypeClass_UNSIGNED_SHORT:
        {
            auto oldint = oldValue.get<sal_uInt16>();
            newValue <<= static_cast<sal_uInt16>(oldint + 5);
            return newValue;
        }
        case TypeClass_SHORT:
        {
            auto oldint = oldValue.get<sal_Int16>();
            newValue <<= static_cast<sal_Int16>(oldint + 5);
            return newValue;
        }
        case TypeClass_LONG:
        {
            auto oldint = oldValue.get<sal_Int32>();
            newValue <<= static_cast<sal_Int32>(oldint + 5);
            return newValue;
        }
        case TypeClass_HYPER:
        {
            auto oldint = oldValue.get<sal_Int64>();
            newValue <<= oldint + 15;
            return newValue;
        }
        case TypeClass_FLOAT:
        {
            auto oldfloat = oldValue.get<float>();
            newValue <<= static_cast<float>(oldfloat + 16.7);
            return newValue;
        }
        case TypeClass_STRING:
        {
            auto oldString = oldValue.get<OUString>();
            newValue <<= oldString + "New";
            return newValue;
        }
        default:
            break;
    }
    if (oldValue.has<css::awt::FontDescriptor>())
    {
        auto _newValue = oldValue.get<css::awt::FontDescriptor>();
        _newValue.CharacterWidth += 5;
        _newValue.CharSet = css::awt::CharSet::ANSI;
        _newValue.Family = css::awt::FontFamily::DECORATIVE;
        _newValue.Height += 2;
        _newValue.Kerning = !_newValue.Kerning;
        _newValue.Name = u"Courier"_ustr;
        _newValue.Orientation += 45;
        _newValue.Pitch = css::awt::FontPitch::VARIABLE;
        _newValue.Slant = css::awt::FontSlant_REVERSE_ITALIC;
        _newValue.Strikeout = css::awt::FontStrikeout::X;
        _newValue.StyleName = u"Bold"_ustr;
        _newValue.Type = css::awt::FontType::SCALABLE;
        _newValue.Underline = css::awt::FontUnderline::BOLDDASHDOTDOT;
        _newValue.Weight += 5;
        _newValue.Width += 6;
        _newValue.WordLineMode = !_newValue.WordLineMode;

        newValue <<= _newValue;
        return newValue;
    }
    assert(false && "missing something in the above code");
    return newValue;
}

CPPUNIT_TEST_FIXTURE(Test, setPropertyValues)
{
    // Load document
    cpo::uno::Sequence<PropertyValue> aLoadArgs{ comphelper::makePropertyValue(u"Hidden"_ustr,
                                                                               false) };
    loadWithParams(u"private:factory/sdraw"_ustr, aLoadArgs);

    OUString objName = u"GroupBox"_ustr;
    Reference<XControlShape> shape
        = insertControlShape(mxComponent, 5000, 7000, 2000, 2000, objName);
    Reference<XMultiPropertySet> xPropSet(shape->getControl(), UNO_QUERY);

    Sequence<OUString> boundPropsToTest = getBoundPropsToTest(xPropSet);

    rtl::Reference<MyChangeListener> ml = new MyChangeListener;
    xPropSet->addPropertiesChangeListener(boundPropsToTest, ml);

    Sequence<Any> gValues = xPropSet->getPropertyValues(boundPropsToTest);
    Sequence<Any> newValue(gValues.getLength());
    // Trying to change all properties
    for (int i = 0; i < boundPropsToTest.getLength(); i++)
    {
        newValue.getArray()[i] = changePValue(gValues[i]);
    }
    xPropSet->setPropertyValues(boundPropsToTest, newValue);

    CPPUNIT_ASSERT_MESSAGE("Listener was not called.", ml->wasListenerCalled());
    xPropSet->removePropertiesChangeListener(ml);
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
