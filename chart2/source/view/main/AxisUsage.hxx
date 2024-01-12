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

#include <sal/types.h>
#include <memory>
#include <map>

#include <VCoordinateSystem.hxx>
#include <AxisHelper.hxx>
#include <ScaleAutomatism.hxx>

namespace chart
{
//first index is the dimension, second index is the axis index that indicates whether this is a main or secondary axis
typedef std::pair<sal_Int32, sal_Int32> tFullAxisIndex;
typedef std::map<VCoordinateSystem*, tFullAxisIndex> tCoordinateSystemMap;

/** This class handles a collection of coordinate systems and is used for
 *  executing some action on all coordinate systems such as
 *  "prepareAutomaticAxisScaling" and "setExplicitScaleAndIncrement".
 *  Moreover it contains the "aAutoScaling" object that is an instance of
 *  the "ScaleAutomatism" class. The initialization of "aAutoScaling" is
 *  performed in the "SeriesPlotterContainer::initAxisUsageList" method and is
 *  used in the "SeriesPlotterContainer::doAutoScaling" for calculating explicit
 *  scale and increment objects (see "SeriesPlotterContainer::doAutoScaling").
 */
class AxisUsage
{
public:
    AxisUsage()
        : aAutoScaling(AxisHelper::createDefaultScale(), Date(Date::SYSTEM))
    {
    }

    void addCoordinateSystem(VCoordinateSystem* pCooSys, sal_Int32 nDimensionIndex,
                             sal_Int32 nAxisIndex)
    {
        if (!pCooSys)
            return;

        tFullAxisIndex aFullAxisIndex(nDimensionIndex, nAxisIndex);
        tCoordinateSystemMap::const_iterator aFound(aCoordinateSystems.find(pCooSys));

        //use one scale only once for each coordinate system
        //main axis are preferred over secondary axis
        //value scales are preferred
        if (aFound != aCoordinateSystems.end())
        {
            sal_Int32 nFoundAxisIndex = aFound->second.second;
            if (nFoundAxisIndex < nAxisIndex)
                return;
            sal_Int32 nFoundDimension = aFound->second.first;
            if (nFoundDimension == 1)
                return;
            if (nFoundDimension < nDimensionIndex)
                return;
        }
        aCoordinateSystems[pCooSys] = std::move(aFullAxisIndex);

        //set maximum scale index
        auto aIter = aMaxIndexPerDimension.find(nDimensionIndex);
        if (aIter != aMaxIndexPerDimension.end())
        {
            sal_Int32 nCurrentMaxIndex = aIter->second;
            if (nCurrentMaxIndex < nAxisIndex)
                aMaxIndexPerDimension[nDimensionIndex] = nAxisIndex;
        }
        else
            aMaxIndexPerDimension[nDimensionIndex] = nAxisIndex;
    }

    std::vector<VCoordinateSystem*> getCoordinateSystems(sal_Int32 nDimensionIndex,
                                                         sal_Int32 nAxisIndex)
    {
        std::vector<VCoordinateSystem*> aRet;

        for (auto const& coordinateSystem : aCoordinateSystems)
        {
            if (coordinateSystem.second.first != nDimensionIndex)
                continue;
            if (coordinateSystem.second.second != nAxisIndex)
                continue;
            aRet.push_back(coordinateSystem.first);
        }

        return aRet;
    }

    sal_Int32 getMaxAxisIndexForDimension(sal_Int32 nDimensionIndex)
    {
        sal_Int32 nRet = -1;
        auto aIter = aMaxIndexPerDimension.find(nDimensionIndex);
        if (aIter != aMaxIndexPerDimension.end())
            nRet = aIter->second;
        return nRet;
    }

    void prepareAutomaticAxisScaling(ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex,
                                     sal_Int32 nAxisIndex)
    {
        std::vector<VCoordinateSystem*> aVCooSysList = getCoordinateSystems(nDimIndex, nAxisIndex);
        for (VCoordinateSystem* pVCoordinateSystem : aVCooSysList)
            pVCoordinateSystem->prepareAutomaticAxisScaling(rScaleAutomatism, nDimIndex,
                                                            nAxisIndex);
    }

    void setExplicitScaleAndIncrement(sal_Int32 nDimIndex, sal_Int32 nAxisIndex,
                                      const ExplicitScaleData& rScale,
                                      const ExplicitIncrementData& rInc)
    {
        std::vector<VCoordinateSystem*> aVCooSysList = getCoordinateSystems(nDimIndex, nAxisIndex);
        for (VCoordinateSystem* pVCoordinateSystem : aVCooSysList)
            pVCoordinateSystem->setExplicitScaleAndIncrement(nDimIndex, nAxisIndex, rScale, rInc);
    }

    ScaleAutomatism aAutoScaling;

private:
    tCoordinateSystemMap aCoordinateSystems;
    std::map<sal_Int32, sal_Int32> aMaxIndexPerDimension;
};

} //end chart2 namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
