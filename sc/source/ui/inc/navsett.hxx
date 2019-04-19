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

#include <tools/solar.h>

#include <o3tl/enumarray.hxx>
#include "content.hxx"

/** Contains settings of the navigator listbox. This includes the expand state
    of each listbox entry and the index of the selected entry and sub entry. */
class ScNavigatorSettings
{
private:
    o3tl::enumarray<ScContentId,bool> maExpandedVec;      /// Array of Booleans for expand state.
    ScContentId                     mnRootSelected;     /// Index of selected root entry.
    sal_uLong                       mnChildSelected;    /// Index of selected child entry.

public:
                                ScNavigatorSettings();

    void                 SetExpanded( ScContentId nIndex, bool bExpand ) { maExpandedVec[ nIndex ] = bExpand; }
    bool                 IsExpanded( ScContentId nIndex ) const { return maExpandedVec[ nIndex ]; }

    void                 SetRootSelected( ScContentId nIndex ) { mnRootSelected = nIndex; }
    ScContentId          GetRootSelected() const { return mnRootSelected; }

    void                 SetChildSelected( sal_uLong nIndex ) { mnChildSelected = nIndex; }
    sal_uLong            GetChildSelected() const { return mnChildSelected; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
