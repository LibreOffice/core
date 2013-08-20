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

#ifndef _SVDLAYER_HXX
#define _SVDLAYER_HXX

#include <tools/stream.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx> // fuer typedef SdrLayerID
#include "svx/svxdllapi.h"
#include <algorithm>

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
    SdrModel*     GetModel() const                            { return pModel; }
    // Einem SdrLayer kann man sagen dass er ein (der) Standardlayer sein soll.
    // Es wird dann laenderspeziefisch der passende Name gesetzt. SetName()
    // setzt das Flag "StandardLayer" ggf. zurueck auf "Userdefined".
    void          SetStandardLayer(bool bStd = true);
    bool          IsStandardLayer() const                     { return nType==1; }
};

// Beim Aendern von Layerdaten muss man derzeit
// noch selbst das Modify-Flag am Model setzen.
#define SDRLAYER_MAXCOUNT 255
class SVX_DLLPUBLIC SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;

protected:
    std::vector<SdrLayer*> aLayer;
    SdrLayerAdmin* pParent; // Der Admin der Seite kennt den Admin des Docs
    SdrModel*      pModel; // zum Broadcasten
    OUString       maControlLayerName;
protected:
    // Eine noch nicht verwendete LayerID raussuchen. Sind bereits alle
    // verbraucht, so gibt's 'ne 0. Wer sicher gehen will, muss vorher
    // GetLayerCount()<SDRLAYER_MAXCOUNT abfragen, denn sonst sind alle
    // vergeben.
    SdrLayerID           GetUniqueLayerID() const;
    void                 Broadcast() const;
public:
    explicit SdrLayerAdmin(SdrLayerAdmin* pNewParent=NULL);
    SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin);
    ~SdrLayerAdmin();
    const SdrLayerAdmin& operator=(const SdrLayerAdmin& rSrcLayerAdmin);
    bool             operator==(const SdrLayerAdmin& rCmpLayerAdmin) const;
    bool             operator!=(const SdrLayerAdmin& rCmpLayerAdmin) const       { return !operator==(rCmpLayerAdmin); }
    SdrLayerAdmin*       GetParent() const                                           { return pParent; }
    void                 SetParent(SdrLayerAdmin* pNewParent)                        { pParent=pNewParent; }
    void                 SetModel(SdrModel* pNewModel);
    SdrModel*            GetModel() const                                            { return pModel; }
    void                 InsertLayer(SdrLayer* pLayer)
    {
        aLayer.push_back(pLayer);
        pLayer->SetModel(pModel);
        Broadcast();
    }
    void                 InsertLayer(SdrLayer* pLayer, sal_uInt16 nPos)
    {
        if(nPos==0xFFFF)
            aLayer.push_back(pLayer);
        else
            aLayer.insert(aLayer.begin() + nPos, pLayer);
        pLayer->SetModel(pModel);
        Broadcast();
    }
    SdrLayer*            RemoveLayer(sal_uInt16 nPos);
    // Alle Layer loeschen
    void               ClearLayer();
    // Neuer Layer wird angelegt und eingefuegt
    SdrLayer*          NewLayer(const OUString& rName, sal_uInt16 nPos=0xFFFF);
    void               DeleteLayer(SdrLayer* pLayer)
    {
        std::vector<SdrLayer*>::iterator it = std::find(aLayer.begin(), aLayer.end(), pLayer);
        if( it == aLayer.end() )
            return;
        aLayer.erase(it);
        delete pLayer;
        Broadcast();
    }
    // Neuer Layer, Name wird aus der Resource geholt
    SdrLayer*          NewStandardLayer(sal_uInt16 nPos=0xFFFF);

    // Iterieren ueber alle Layer
    sal_uInt16             GetLayerCount() const                                         { return sal_uInt16(aLayer.size()); }
    SdrLayer*          GetLayer(sal_uInt16 i)                                            { return aLayer[i]; }
    const SdrLayer*    GetLayer(sal_uInt16 i) const                                      { return aLayer[i]; }

    sal_uInt16             GetLayerPos(SdrLayer* pLayer) const;

    SdrLayer* GetLayer(const OUString& rName, bool bInherited);
    const SdrLayer* GetLayer(const OUString& rName, bool bInherited) const;
    SdrLayerID GetLayerID(const OUString& rName, bool bInherited) const;
          SdrLayer*    GetLayerPerID(sal_uInt16 nID)                                     { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayerPerID(nID)); }
    const SdrLayer*    GetLayerPerID(sal_uInt16 nID) const;

    void SetControlLayerName(const OUString& rNewName);
    const OUString& GetControlLayerName() const { return maControlLayerName; }
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

#endif //_SVDLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
