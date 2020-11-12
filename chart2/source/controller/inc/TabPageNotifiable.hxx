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

// color to use as foreground for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR COL_WHITE
// color to use as background for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR Color(0xff6563)

class BuilderPage;

namespace chart
{
class TabPageNotifiable
{
public:
    virtual void setInvalidPage(BuilderPage* pTabPage) = 0;
    virtual void setValidPage(BuilderPage* pTabPage) = 0;

protected:
    ~TabPageNotifiable() {}
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
