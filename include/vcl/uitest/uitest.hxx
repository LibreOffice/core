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

class UIObject;

class UITEST_DLLPUBLIC UITest
{
public:

    static void executeCommand(const OUString& rCommand);

    static std::unique_ptr<UIObject> getFocusTopWindow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
