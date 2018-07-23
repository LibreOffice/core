/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include <vcl/dllapi.h>

#include <memory>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class UIObject;

class UITEST_DLLPUBLIC UITest
{
public:

    static bool executeCommand(const OUString& rCommand,
        const css::uno::Sequence< css::beans::PropertyValue >& rArgs = css::uno::Sequence< css::beans::PropertyValue >());

    static bool executeDialog(const OUString& rCommand);

    static std::unique_ptr<UIObject> getFocusTopWindow();

    static std::unique_ptr<UIObject> getFloatWindow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
