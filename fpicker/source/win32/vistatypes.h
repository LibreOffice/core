/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#pragma once

#include <shobjidl.h>
#include <systools/win32/comtools.hxx>

namespace fpicker{
namespace win32{
namespace vista{


// types, const etcpp.


typedef sal::systools::COMReference<IFileDialog> TFileDialog;
typedef sal::systools::COMReference<IFileOpenDialog> TFileOpenDialog;
typedef sal::systools::COMReference<IFileSaveDialog> TFileSaveDialog;
typedef sal::systools::COMReference<IFileDialogEvents> TFileDialogEvents;
typedef sal::systools::COMReference<IFileDialogCustomize> TFileDialogCustomize;

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
