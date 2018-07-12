/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/dllapi.h>

#include <tools/stream.hxx>
#include <vcl/ctrl.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

typedef std::map<const OUString, OUString> StringMap;

class UITEST_DLLPUBLIC UITestLogger
{
private:

    SvFileStream maStream;

    bool mbValid;

public:

    UITestLogger();

    void logCommand(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rArgs);

    void logAction(VclPtr<Control> const & xUIElement, VclEventId nEvent);

    void log(const OUString& rString);

    void logKeyInput(VclPtr<vcl::Window> const & xUIElement, const KeyEvent& rEvent);

    void logSwEditWinEvent(const OUString& rAction, const StringMap& rParameters = StringMap());

    void logScGridWinEvent(const OUString& rAction, const StringMap& rParameters = StringMap());

    void logImpressWinEvent(const OUString& rAction, const StringMap& rParameters = StringMap());

    void logStarMathEvent(const OUString& rID, const OUString& rAction);

    void logObjectSelection(const OUString& rID);

    static UITestLogger& getInstance();

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
