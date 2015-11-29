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

#ifndef INCLUDED_SVX_SVDLAYER_HXX
#define INCLUDED_SVX_SVDLAYER_HXX

#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>
#include <algorithm>
#include <vector>

/**
 * Note on the layer mix with symbolic/ID-based interface:
 * You create a new layer with
 *    pLayerAdmin->NewLayer("A new layer");
 * This layer is automatically appended to the end of the list.
 *
 * The same holds true for layer sets.
 *
 * The interface for SdrLayerSet is based on LayerIDs. The app must get
 * an ID for it at the SdrLayerAdmin, like so:
 *   SdrLayerID nLayerID=pLayerAdmin->GetLayerID("A new layer");
 *
 * If the layer cannot be found, SDRLAYER_NOTFOUND is returned.
 * The methods with the ID interface usually handle that error in a
 * meaningful way.
 * If you not only got a name, but even a SdrLayer*, you can get the ID
 * much faster via the layer directly.
 *
 * @param bInherited:
 * TRUE If the layer/layer set cannot be found, we examine the parent layer admin,
 *      whether there's a corresponding definition
 * FALSE We only search this layer admin
 *
 * Every page's layer admin has a parent layer admin (the model's). The model
 * itself does not have a parent.
 */

class SdrModel;

class SVX_DLLPUBLIC SdrLayer
{
    friend class SdrLayerAdmin;

    OUString maName;
    OUString maTitle;
    OUString maDescription;
    SdrModel*  pModel; // For broadcasting
    sal_uInt16 nType;  // 0= userdefined, 1= default layer
    SdrLayerID nID;

    SdrLayer(SdrLayerID nNewID, const OUString& rNewName);

public:
    bool operator==(const SdrLayer& rCmpLayer) const;
    bool operator!=(const SdrLayer& rCmpLayer) const { return !operator==(rCmpLayer); }

    void SetName(const OUString& rNewName);
    const OUString& GetName() const { return maName; }

    void SetTitle(const OUString& rTitle) { maTitle = rTitle; }
    const OUString& GetTitle() const { return maTitle; }

    void SetDescription(const OUString& rDesc) { maDescription = rDesc; }
    const OUString& GetDescription() const { return maDescription; }

    SdrLayerID    GetID() const                               { return nID; }
    void          SetModel(SdrModel* pNewModel)               { pModel=pNewModel; }
    // A SdrLayer should be considered the standard Layer. It shall then set the
    // appropriate country-specific name. SetName() sets the "StandardLayer" flag
    // and if necessary returns "Userdefined".
    void          SetStandardLayer(bool bStd = true);
};

// When Changing the layer data you currently have to set the Modify flag manually
#define SDRLAYER_MAXCOUNT 255
class SVX_DLLPUBLIC SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;

protected:
    std::vector<SdrLayer*> aLayer;
    SdrLayerAdmin* pParent; // The page's admin knows the doc's admin
    SdrModel* pModel; // For broadcasting
    OUString maControlLayerName;
protected:
    // Find a LayerID which is not in use yet. If all have been used up,
    // we return 0.
    // If you want to play safe, check GetLayerCount()<SDRLAYER_MAXCOUNT
    // first, else all are given away already.
    SdrLayerID         GetUniqueLayerID() const;
    void               Broadcast() const;
public:
    explicit SdrLayerAdmin(SdrLayerAdmin* pNewParent=nullptr);
    SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin);
    ~SdrLayerAdmin();
    const SdrLayerAdmin& operator=(const SdrLayerAdmin& rSrcLayerAdmin);
    bool               operator==(const SdrLayerAdmin& rCmpLayerAdmin) const;
    bool               operator!=(const SdrLayerAdmin& rCmpLayerAdmin) const       { return !operator==(rCmpLayerAdmin); }
    void               SetParent(SdrLayerAdmin* pNewParent)                        { pParent=pNewParent; }
    void               SetModel(SdrModel* pNewModel);
    void               InsertLayer(SdrLayer* pLayer, sal_uInt16 nPos)
    {
        if(nPos==0xFFFF)
            aLayer.push_back(pLayer);
        else
            aLayer.insert(aLayer.begin() + nPos, pLayer);
        pLayer->SetModel(pModel);
        Broadcast();
    }
    SdrLayer*          RemoveLayer(sal_uInt16 nPos);

    // Delete the entire layer
    void               ClearLayer();

    // New layer is created and inserted
    SdrLayer*          NewLayer(const OUString& rName, sal_uInt16 nPos=0xFFFF);

    // New layer, name is retrieved from the resource
    SdrLayer*          NewStandardLayer(sal_uInt16 nPos=0xFFFF);

    // Iterate over all layers
    sal_uInt16         GetLayerCount() const                                         { return sal_uInt16(aLayer.size()); }

    SdrLayer*          GetLayer(sal_uInt16 i)                                        { return aLayer[i]; }
    const SdrLayer*    GetLayer(sal_uInt16 i) const                                  { return aLayer[i]; }

    sal_uInt16         GetLayerPos(SdrLayer* pLayer) const;

    SdrLayer*          GetLayer(const OUString& rName, bool bInherited);
    const SdrLayer*    GetLayer(const OUString& rName, bool bInherited) const;
    SdrLayerID         GetLayerID(const OUString& rName, bool bInherited) const;
    SdrLayer*          GetLayerPerID(sal_uInt16 nID) { return const_cast<SdrLayer*>(const_cast<const SdrLayerAdmin*>(this)->GetLayerPerID(nID)); }
    const SdrLayer*    GetLayerPerID(sal_uInt16 nID) const;

    void               SetControlLayerName(const OUString& rNewName);
    const OUString&    GetControlLayerName() const { return maControlLayerName; }
};

#endif // INCLUDED_SVX_SVDLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
