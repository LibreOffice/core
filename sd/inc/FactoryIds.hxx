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

#ifndef INCLUDED_SD_INC_FACTORYIDS_HXX
#define INCLUDED_SD_INC_FACTORYIDS_HXX

namespace sd {

/** This are ids used by SfxTopFrame::Create() or CreateViewFrame() to
    select a factory to create an instance of an SfxViewShell super
    class.  This allows the caller to create a view shell directly
    with a certain view instead of first create a default view and
    then switch to the desired view.
*/
enum ViewShellFactoryIds
{
    IMPRESS_FACTORY_ID = 1,
    DRAW_FACTORY_ID = 1,
    SLIDE_SORTER_FACTORY_ID = 2,
    OUTLINE_FACTORY_ID = 3,
    PRESENTATION_FACTORY_ID = 4
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
