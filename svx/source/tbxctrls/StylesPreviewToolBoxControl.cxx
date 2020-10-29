/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <StylesPreviewToolBoxControl.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

StylesPreviewToolBoxControl::StylesPreviewToolBoxControl() {}

StylesPreviewToolBoxControl::~StylesPreviewToolBoxControl() {}

void SAL_CALL
StylesPreviewToolBoxControl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    svt::ToolboxController::initialize(rArguments);

    if (m_xFrame.is())
    {
        InitializeStyles(m_xFrame->getController()->getModel());

        m_xDispatchProvider = css::uno::Reference<css::frame::XDispatchProvider>(
            m_xFrame->getController(), css::uno::UNO_QUERY);
    }
}

void SAL_CALL StylesPreviewToolBoxControl::dispose()
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    m_xWeldBox.reset();
}

void StylesPreviewToolBoxControl::InitializeStyles(
    const css::uno::Reference<css::frame::XModel>& xModel)
{
    m_aDefaultStyles.clear();

    //now convert the default style names to the localized names
    try
    {
        css::uno::Reference<css::style::XStyleFamiliesSupplier> xStylesSupplier(
            xModel, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::lang::XServiceInfo> xServices(xModel, css::uno::UNO_QUERY_THROW);
        if (xServices->supportsService("com.sun.star.text.TextDocument"))
        {
            css::uno::Reference<css::container::XNameAccess> xParaStyles;
            xStylesSupplier->getStyleFamilies()->getByName("ParagraphStyles") >>= xParaStyles;
            static const std::vector<OUString> aWriterStyles
                = { "Standard",  "Text body", "Heading 1", "Heading 2",  "Heading 3",
                    "Heading 4", "Title",     "Subtitle",  "Quotations", "Preformatted Text" };
            for (const OUString& aStyle : aWriterStyles)
            {
                try
                {
                    css::uno::Reference<css::beans::XPropertySet> xStyle;
                    xParaStyles->getByName(aStyle) >>= xStyle;
                    OUString sName;
                    xStyle->getPropertyValue("DisplayName") >>= sName;
                    if (!sName.isEmpty())
                        m_aDefaultStyles.push_back(std::pair<OUString, OUString>(aStyle, sName));
                }
                catch (const css::container::NoSuchElementException&)
                {
                }
                catch (const css::uno::Exception&)
                {
                }
            }
        }
        else if (xServices->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
        {
            static const char* aCalcStyles[] = { "Default", "Heading1", "Result", "Result2" };
            css::uno::Reference<css::container::XNameAccess> xCellStyles;
            xStylesSupplier->getStyleFamilies()->getByName("CellStyles") >>= xCellStyles;
            for (const char* pCalcStyle : aCalcStyles)
            {
                try
                {
                    const OUString sStyleName(OUString::createFromAscii(pCalcStyle));
                    if (xCellStyles->hasByName(sStyleName))
                    {
                        css::uno::Reference<css::beans::XPropertySet> xStyle(
                            xCellStyles->getByName(sStyleName), css::uno::UNO_QUERY_THROW);
                        OUString sName;
                        xStyle->getPropertyValue("DisplayName") >>= sName;
                        if (!sName.isEmpty())
                        {
                            m_aDefaultStyles.push_back(
                                std::pair<OUString, OUString>(sStyleName, sName));
                        }
                    }
                }
                catch (const css::uno::Exception&)
                {
                }
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        OSL_FAIL("error while initializing style names");
    }
}

void SAL_CALL StylesPreviewToolBoxControl::update() {}

void StylesPreviewToolBoxControl::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/) {}

css::uno::Reference<css::awt::XWindow>
StylesPreviewToolBoxControl::createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    css::uno::Reference<css::awt::XWindow> xItemWindow;

    /* TODO
    if (m_pBuilder)
    {
        SolarMutexGuard aSolarMutexGuard;

        std::unique_ptr<weld::Container> xWidget(*m_pBuilder);

        xItemWindow
            = css::uno::Reference<css::awt::XWindow>(new weld::TransportAsXWindow(xWidget.get()));

        m_xWeldBox.reset(new StylesPreviewWindow_Base(std::move(xWidget)));
        m_pBox = m_xWeldBox.get();
    }
    else
    */
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
        if (pParent)
        {
            SolarMutexGuard aSolarMutexGuard;

            m_xVclBox = VclPtr<StylesPreviewWindow_Impl>::Create(pParent, m_aDefaultStyles,
                                                                 m_xDispatchProvider);
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox);
        }
    }

    return xItemWindow;
}

OUString StylesPreviewToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.StylesPreviewToolBoxControl";
}

sal_Bool StylesPreviewToolBoxControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> StylesPreviewToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_StylesPreviewToolBoxControl_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new StylesPreviewToolBoxControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
