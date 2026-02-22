/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/color.hxx>
#include <vcl/ColorDialog.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/weld.hxx>

namespace
{
class ColorPicker : public comphelper::WeakComponentImplHelper<
                        css::lang::XServiceInfo, css::ui::dialogs::XExecutableDialog,
                        css::lang::XInitialization, css::beans::XPropertyAccess>
{
public:
    explicit ColorPicker();

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPropertyAccess
    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& rProperties) override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle(const OUString& rTitle) override;
    virtual sal_Int16 SAL_CALL execute() override;

private:
    Color m_aColor;
    css::uno::Reference<css::awt::XWindow> m_xParent;
};

constexpr OUString COLOR_PROPERTY_NAME = u"Color"_ustr;

ColorPicker::ColorPicker()
    : m_aColor(COL_BLACK)
{
}

void SAL_CALL ColorPicker::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    if (rArguments.getLength() == 1)
    {
        rArguments[0] >>= m_xParent;
    }
}

OUString SAL_CALL ColorPicker::getImplementationName()
{
    return u"com.sun.star.cui.ColorPicker"_ustr;
}

sal_Bool SAL_CALL ColorPicker::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ColorPicker::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.dialogs.ColorPicker"_ustr };
}

css::uno::Sequence<css::beans::PropertyValue> SAL_CALL ColorPicker::getPropertyValues()
{
    css::uno::Sequence<css::beans::PropertyValue> aProps{ comphelper::makePropertyValue(
        COLOR_PROPERTY_NAME, m_aColor) };
    return aProps;
}

void SAL_CALL
ColorPicker::setPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& rProperties)
{
    for (const css::beans::PropertyValue& rProp : rProperties)
    {
        if (rProp.Name == COLOR_PROPERTY_NAME)
            rProp.Value >>= m_aColor;
    }
}

void SAL_CALL ColorPicker::setTitle(const OUString&) {}

sal_Int16 SAL_CALL ColorPicker::execute()
{
    ColorDialog aColorDialog(Application::GetFrameWeld(m_xParent));
    const int nRet = aColorDialog.Execute();
    if (nRet == RET_OK)
        m_aColor = aColorDialog.GetColor();
    return nRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_cui_ColorPicker_get_implementation(css::uno::XComponentContext*,
                                                const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new ColorPicker);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
