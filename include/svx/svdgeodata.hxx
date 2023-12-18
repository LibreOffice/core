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

#include <svx/svxdllapi.h>
#include <svx/svdtypes.hxx>
#include <svx/svdglue.hxx>
#include <tools/gen.hxx>
#include <optional>

/**
 * All geometrical data of an arbitrary object for use in undo/redo
 */
class SVXCORE_DLLPUBLIC SdrObjGeoData
{
public:
    tools::Rectangle aBoundRect;
    Point aAnchor;
    std::optional<SdrGluePointList> moGluePoints;
    bool bMovProt;
    bool bSizProt;
    bool bNoPrint;
    bool bClosedObj;
    bool mbVisible;
    SdrLayerID mnLayerID;

public:
    SdrObjGeoData();
    virtual ~SdrObjGeoData();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
