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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDRAWMODELMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDRAWMODELMANAGER_HXX

#include <IDocumentDrawModelAccess.hxx>
#include <svx/svdtypes.hxx>
#include <memory>
#include <drawdoc.hxx>

class SdrPageView;
class SwDoc;

namespace sw
{

class DocumentDrawModelManager : public IDocumentDrawModelAccess
{
public:

    DocumentDrawModelManager( SwDoc& i_rSwdoc );

    void InitDrawModel();
    void ReleaseDrawModel();
    void DrawNotifyUndoHdl();

    //IDocumentDrawModelAccess
    virtual const SwDrawModel* GetDrawModel() const override;
    virtual SwDrawModel* GetDrawModel() override;
    virtual SwDrawModel* MakeDrawModel_() override;
    virtual SwDrawModel* GetOrCreateDrawModel() override;
    virtual SdrLayerID GetHeavenId() const override;
    virtual SdrLayerID GetHellId() const override;
    virtual SdrLayerID GetControlsId() const override;
    virtual SdrLayerID GetInvisibleHeavenId() const override;
    virtual SdrLayerID GetInvisibleHellId() const override;
    virtual SdrLayerID GetInvisibleControlsId() const override;

    virtual void NotifyInvisibleLayers( SdrPageView& _rSdrPageView ) override;

    virtual bool IsVisibleLayerId( SdrLayerID _nLayerId ) const override;

    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne( SdrLayerID _nVisibleLayerId ) override;

    virtual bool Search(const SwPaM& rPaM, const SvxSearchItem& rSearchItem) override;

private:

    DocumentDrawModelManager(DocumentDrawModelManager const&) = delete;
    DocumentDrawModelManager& operator=(DocumentDrawModelManager const&) = delete;

    SwDoc& m_rDoc;

    std::unique_ptr<SwDrawModel> mpDrawModel;

    /** Draw Model Layer IDs
     * LayerIds, Heaven == above document
     *           Hell   == below document
     *         Controls == at the very top
     */
    SdrLayerID mnHeaven;
    SdrLayerID mnHell;
    SdrLayerID mnControls;
    SdrLayerID mnInvisibleHeaven;
    SdrLayerID mnInvisibleHell;
    SdrLayerID mnInvisibleControls;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
