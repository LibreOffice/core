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

#include <svx/svdtypes.hxx>

class SwDrawModel;
class SwPaM;
class SdrPageView;
class SvxSearchItem;

class IDocumentDrawModelAccess
{
public:
    /** Draw Model and id accessors
     */
    virtual const SwDrawModel* GetDrawModel() const = 0;
    virtual SwDrawModel* GetDrawModel() = 0;
    virtual SwDrawModel* MakeDrawModel_() = 0;
    virtual SwDrawModel* GetOrCreateDrawModel() = 0;
    virtual SdrLayerID GetHeavenId() const = 0;
    virtual SdrLayerID GetHellId() const = 0;
    virtual SdrLayerID GetHeaderFooterHellId() const = 0;
    virtual SdrLayerID GetControlsId() const = 0;
    virtual SdrLayerID GetInvisibleHeavenId() const = 0;
    virtual SdrLayerID GetInvisibleHellId() const = 0;
    virtual SdrLayerID GetInvisibleControlsId() const = 0;

    /** method to notify drawing page view about the invisible layers */
    virtual void NotifyInvisibleLayers(SdrPageView& _rSdrPageView) = 0;

    /** method to determine, if a layer ID belongs to the visible ones.
        Note: If given layer ID is unknown, method asserts and returns <false>.

        @param _nLayerId
        input parameter - layer ID, which has to be checked, if it belongs to
        the visible ones.

        @return bool, indicating, if given layer ID belongs to the visible ones.
    */
    virtual bool IsVisibleLayerId(SdrLayerID _nLayerId) const = 0;

    /** method to determine, if the corresponding invisible layer ID for a visible one.

        Note: If given layer ID is an invisible one, method returns given layer ID.
        Note: If given layer ID is unknown, method returns given layer ID.

        @param _nVisibleLayerId
        input parameter - visible layer ID for which the corresponding
        invisible one has to be returned.

        @return sal_Int8, invisible layer ID corresponding to given layer ID
    */
    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne(SdrLayerID _nVisibleLayerId) = 0;

    /// Searches text in shapes anchored inside rPaM.
    virtual bool Search(const SwPaM& rPaM, const SvxSearchItem& rSearchItem) = 0;

protected:
    virtual ~IDocumentDrawModelAccess(){};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
