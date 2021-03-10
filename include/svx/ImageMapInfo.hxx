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

#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>
#include <vcl/imap.hxx>

#define SVX_IMAPINFO_ID 2

class SVXCORE_DLLPUBLIC SvxIMapInfo final : public SdrObjUserData, public SfxListener
{
    ImageMap aImageMap;

public:
    SvxIMapInfo(const ImageMap& rImageMap)
        : SdrObjUserData(SdrInventor::StarDrawUserData, SVX_IMAPINFO_ID)
        , aImageMap(rImageMap){};

    SvxIMapInfo(const SvxIMapInfo& rIMapInfo)
        : SdrObjUserData(SdrInventor::StarDrawUserData, SVX_IMAPINFO_ID)
        , SfxListener()
        , aImageMap(rIMapInfo.aImageMap){};

    virtual std::unique_ptr<SdrObjUserData> Clone(SdrObject*) const override
    {
        return std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(*this));
    }

    void SetImageMap(const ImageMap& rIMap) { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const { return aImageMap; }

    static SvxIMapInfo* GetIMapInfo(const SdrObject* pObject);
    static IMapObject* GetHitIMapObject(const SdrObject* pObj, const Point& rWinPoint,
                                        const OutputDevice* pCmpWnd = nullptr);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
