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

#include <tools/stream.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>
#include <algorithm>
#include <vector>

class SdrModel;

class SVX_DLLPUBLIC SdrLayer
{
    friend class SdrLayerAdmin;

    OUString maName;
    OUString maTitle;
    OUString maDescription;
    SdrModel*  pModel; // zum Broadcasten
    sal_uInt16     nType;  // 0=Userdefined,1=Standardlayer
    SdrLayerID nID;

    SdrLayer(SdrLayerID nNewID, const OUString& rNewName);

public:
    bool      operator==(const SdrLayer& rCmpLayer) const;
    bool      operator!=(const SdrLayer& rCmpLayer) const { return !operator==(rCmpLayer); }

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

// When Changing the layer data you currently have to set the Modify-Flag
// manually
#define SDRLAYER_MAXCOUNT 255
class SVX_DLLPUBLIC SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;

protected:
    std::vector<SdrLayer*> aLayer;
    SdrLayerAdmin* pParent; // Der Admin der Seite kennt den Admin des Docs
    SdrModel*      pModel; // for broadcasting
    OUString       maControlLayerName;
protected:
    // Eine noch nicht verwendete LayerID raussuchen. Sind bereits alle
    // verbraucht, so gibt's 'ne 0. Wer sicher gehen will, muss vorher
    // GetLayerCount()<SDRLAYER_MAXCOUNT abfragen, denn sonst sind alle
    // vergeben.
    SdrLayerID         GetUniqueLayerID() const;
    void               Broadcast() const;
public:
    explicit SdrLayerAdmin(SdrLayerAdmin* pNewParent=NULL);
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
    SdrLayer*          GetLayerPerID(sal_uInt16 nID)                                     { return const_cast<SdrLayer*>(const_cast<const SdrLayerAdmin*>(this)->GetLayerPerID(nID)); }
    const SdrLayer*    GetLayerPerID(sal_uInt16 nID) const;

    void               SetControlLayerName(const OUString& rNewName);
    const OUString&    GetControlLayerName() const { return maControlLayerName; }
};

/*
Anmerkung zu den Layer - Gemischt symbolisch/ID-basierendes Interface
    Einen neuen Layer macht man sich mit:
      pLayerAdmin->NewLayer("Der neue Layer");
    Der Layer wird dann automatisch an das Ende der Liste angehaengt.
    Entsprechdes gilt fuer Layersets gleichermassen.
    Das Interface am SdrLayerSet basiert auf LayerID's. Die App muss sich
    dafuer am SdrLayerAdmin eine ID abholen:
        SdrLayerID nLayerID=pLayerAdmin->GetLayerID("Der neue Layer");
    Wird der Layer nicht gefunden, so liefert die Methode SDRLAYER_NOTFOUND
    zurueck. Die Methoden mit ID-Interface fangen diesen Wert jedoch i.d.R
    sinnvoll ab.
    Hat man nicht nur den Namen, sondern gar einen SdrLayer*, so kann man
    sich die ID natuerlich wesentlich schneller direkt vom Layer abholen.
bInherited:
    TRUE: Wird der Layer/LayerSet nicht gefunden, so wird im Parent-LayerAdmin
          nachgesehen, ob es dort einen entsprechende Definition gibt.
    FALSE: Es wird nur dieser LayerAdmin durchsucht.
    Jeder LayerAdmin einer Seite hat einen Parent-LayerAdmin, n?mlich den des
    Model. Das Model selbst hat keinen Parent.
*/

#endif // INCLUDED_SVX_SVDLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
