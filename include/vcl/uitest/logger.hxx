/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UITEST_LOGGER_HXX
#define INCLUDED_VCL_UITEST_LOGGER_HXX

#include <sal/config.h>

#include <string_view>

#include <vcl/dllapi.h>

#include <tools/stream.hxx>
#include <vcl/vclevent.hxx>

namespace com::sun::star::beans
{
struct PropertyValue;
}
namespace com::sun::star::uno
{
template <class E> class Sequence;
}
struct EventDescription;
class Control;
class KeyEvent;

class UITEST_DLLPUBLIC UITestLogger
{
private:
    SvFileStream maStream;

    bool mbValid;

    OUString app_name;

public:
    UITestLogger();

    void logCommand(const OUString& rAction,
                    const css::uno::Sequence<css::beans::PropertyValue>& rArgs);

    void logAction(VclPtr<Control> const& xUIElement, VclEventId nEvent);

    void logAction(vcl::Window* const& xUIWin, VclEventId nEvent);

    void log(std::u16string_view rString);

    void logKeyInput(VclPtr<vcl::Window> const& xUIElement, const KeyEvent& rEvent);

    void logEvent(const EventDescription& rDescription);

    static UITestLogger& getInstance();

    void setAppName(OUString name) { app_name = name; }

    const OUString& getAppName() const { return app_name; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
