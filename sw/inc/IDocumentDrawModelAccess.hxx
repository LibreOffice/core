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

#ifndef INCLUDED_SW_INC_IDOCUMENTDRAWMODELACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTDRAWMODELACCESS_HXX

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
    virtual SwDrawModel* _MakeDrawModel() = 0;
    virtual SwDrawModel* GetOrCreateDrawModel() = 0;
    virtual SdrLayerID GetHeavenId() const = 0;
    virtual SdrLayerID GetHellId() const = 0;
    virtual SdrLayerID GetControlsId() const = 0;
    virtual SdrLayerID GetInvisibleHeavenId() const = 0;
    virtual SdrLayerID GetInvisibleHellId() const = 0;
    virtual SdrLayerID GetInvisibleControlsId() const = 0;

    /** method to notify drawing page view about the invisible layers
        @author OD
    */
    virtual void NotifyInvisibleLayers( SdrPageView& _rSdrPageView ) = 0;

    /** method to determine, if a layer ID belongs to the visible ones.
        Note: If given layer ID is unknown, method asserts and returns <false>.
        @author OD

        @param _nLayerId
        input parameter - layer ID, which has to be checked, if it belongs to
        the visible ones.

        @return bool, indicating, if given layer ID belongs to the visible ones.
    */
    virtual bool IsVisibleLayerId( const SdrLayerID& _nLayerId ) const = 0;

    /** method to determine, if the corresponding visible layer ID for a invisible one.

        Note: If given layer ID is a visible one, method returns given layer ID.
        Note: If given layer ID is unknown, method returns given layer ID.

        @author OD

        @param _nInvisibleLayerId
        input parameter - invisible layer ID for which the corresponding
        visible one has to be returned.

        @return sal_Int8, visible layer ID corresponding to given layer ID
    */
    virtual SdrLayerID GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId ) = 0;

    /** method to determine, if the corresponding invisible layer ID for a visible one.

        Note: If given layer ID is a invisible one, method returns given layer ID.
        Note: If given layer ID is unknown, method returns given layer ID.

        @author OD

        @param _nVisibleLayerId
        input parameter - visible layer ID for which the corresponding
        invisible one has to be returned.

        @return sal_Int8, invisible layer ID corresponding to given layer ID
    */
    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId ) = 0;

    /// Searches text in shapes anchored inside rPaM.
    virtual bool Search(const SwPaM& rPaM, const SvxSearchItem& rSearchItem) = 0;

protected:

    virtual ~IDocumentDrawModelAccess() {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
