/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UITEST_UITEST_HXX
#define INCLUDED_VCL_UITEST_UITEST_HXX

#include <rtl/ustring.hxx>

#include <memory>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::uno { template <typename > class Sequence; }

class UIObject;

class UITest
{
public:

    static bool executeCommand(const OUString& rCommand);

    static bool executeCommandWithParameters(const OUString& rCommand,
        const css::uno::Sequence< css::beans::PropertyValue >& rArgs);

    static bool executeDialog(const OUString& rCommand);

    static std::unique_ptr<UIObject> getFocusTopWindow();

    static std::unique_ptr<UIObject> getFloatWindow();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
