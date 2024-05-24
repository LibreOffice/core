/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/svapp.hxx>

#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

#include "atspiwrapper.hxx"

/**
 * @brief Base class for AT-SPI2 tests.
 *
 * This provides the basis for interacting with AT-SPI2, including getting the object representing
 * our application, and facility for obtaining a top-level window of that app.
 */
class Atspi2TestBase : public test::AccessibleTestBase
{
protected:
    Atspi::Accessible m_pAtspiApp;

    static Atspi::Accessible getApp(const std::string_view appName)
    {
        std::cout << "Looking for AT-SPI application \"" << appName << "\"" << std::endl;
        const auto nDesktops = atspi_get_desktop_count();
        for (auto desktopId = decltype(nDesktops){ 0 }; desktopId < nDesktops; desktopId++)
        {
            Atspi::Accessible desktop(atspi_get_desktop(desktopId));

            for (auto&& child : desktop)
            {
                if (!child) // is that useful?
                    continue;
                if (child.getRole() != ATSPI_ROLE_APPLICATION)
                    continue;
                const auto name = child.getName();
                std::cout << "Found desktop child: " << name << std::endl;
                if (appName != name)
                {
                    continue;
                }
                return std::move(child);
            }
        }
        return nullptr;
    }

    static Atspi::Accessible getSelfApp()
    {
        const auto appFileName = Application::GetAppFileName();
        const auto slash = appFileName.lastIndexOf('/');
        const auto baseName = (slash >= 0) ? OUString(appFileName.subView(slash + 1)) : appFileName;
        return getApp(baseName.getLength() > 0 ? baseName.toUtf8().getStr() : "cppunittester");
    }

protected:
    Atspi::Accessible getWindow(const std::string_view windowName)
    {
        for (auto&& child : m_pAtspiApp)
        {
            const auto name = child.getName();
            std::cout << "Found window: " << name << std::endl;
            if (windowName == name)
                return std::move(child);
        }
        return nullptr;
    }

public:
    Atspi2TestBase()
    {
        if (!atspi_is_initialized())
            atspi_init();

        // increase timeout to avoid spurious CI failures
        atspi_set_timeout(10000, 15000);
    }

    virtual void setUp() override
    {
        test::AccessibleTestBase::setUp();

        AccessibilityTools::Await([this]() {
            m_pAtspiApp = getSelfApp();
            return bool(m_pAtspiApp);
        });
        assert(m_pAtspiApp);
    }
};
