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

#include <tools/long.hxx>
#include <vcl/vclptr.hxx>

class OutputDevice;

class ScGridMerger
{
private:
    VclPtr<OutputDevice> pDev;
    tools::Long            nOneX;
    tools::Long            nOneY;
    tools::Long            nFixStart;
    tools::Long            nFixEnd;
    tools::Long            nVarStart;
    tools::Long            nVarDiff;
    tools::Long            nCount;
    bool            bVertical;
    bool            bOptimize;

    void        AddLine( tools::Long nStart, tools::Long nEnd, tools::Long nPos );

public:
                ScGridMerger( OutputDevice* pOutDev, tools::Long nOnePixelX, tools::Long nOnePixelY );
                ~ScGridMerger();

    void        AddHorLine(bool bWorksInPixels, tools::Long nX1, tools::Long nX2, tools::Long nY, bool bDashed = false);
    void        AddVerLine(bool bWorksInPixels, tools::Long nX, tools::Long nY1, tools::Long nY2, bool bDashed = false);
    void        Flush();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
