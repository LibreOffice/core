/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

namespace jsdialog
{
bool isBuilderEnabled(const OUString& rUIFile, bool bMobile);
bool isBuilderEnabledForPopup(const OUString& rUIFile);
bool isBuilderEnabledForSidebar(const OUString& rUIFile);
bool isInterimBuilderEnabledForNotebookbar(const OUString& rUIFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
