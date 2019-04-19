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

#include <sfx2/shell.hxx>

namespace sd {

/** This are ids used by SfxTopFrame::Create() or CreateViewFrame() to
    select a factory to create an instance of an SfxViewShell super
    class.  This allows the caller to create a view shell directly
    with a certain view instead of first create a default view and
    then switch to the desired view.
*/
constexpr auto IMPRESS_FACTORY_ID         = SfxInterfaceId(1);
constexpr auto DRAW_FACTORY_ID            = SfxInterfaceId(1);
constexpr auto SLIDE_SORTER_FACTORY_ID    = SfxInterfaceId(2);
constexpr auto OUTLINE_FACTORY_ID         = SfxInterfaceId(3);
constexpr auto PRESENTATION_FACTORY_ID     = SfxInterfaceId(4);

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
