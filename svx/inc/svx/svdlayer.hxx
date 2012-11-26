/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDLAYER_HXX
#define _SVDLAYER_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx> // fuer typedef SdrLayerID
#include "svx/svxdllapi.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrModel;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrLayer
{
private:
protected:
    String          maName;

    // title and description set and get in SD over UNO API
    String      maTitle;
    String      maDescription;

    SdrModel&       mrModel; // zum Broadcasten
    SdrLayerID      mnID;

    /// bitfield
    bool            mbStandardlayer : 1;  // Userdefined or Standardlayer // 0=Userdefined,1=Standardlayer

protected:
    void SetID(SdrLayerID nNewID) { if(mnID != nNewID) mnID = nNewID; }

public:
    SdrLayer(SdrLayerID nNewID, const String& rNewName, SdrModel& rSdrModel);

    bool      operator==(const SdrLayer& rCmpLayer) const;
    bool      operator!=(const SdrLayer& rCmpLayer) const { return !operator==(rCmpLayer); }

    void          SetName(const String& rNewName);
    const String& GetName() const { return maName; }

    void          SetTitle(const String& rTitle) { maTitle = rTitle; }
    const String& GetTitle() const { return maTitle; }
    void          SetDescription(const String& rDesc) { maDescription = rDesc; }
    const String& GetDescription() const { return maDescription; }

    SdrLayerID GetID() const { return mnID; }
    SdrModel& GetSdrModel() const { return mrModel; }

    // Einem SdrLayer kann man sagen dass er ein (der) Standardlayer sein soll.
    // Es wird dann laenderspeziefisch der passende Name gesetzt. SetName()
    // setzt das Flag "StandardLayer" ggf. zurueck auf "Userdefined".
    void SetStandardLayer(bool bStd = true);
    bool IsStandardLayer() const { return mbStandardlayer; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Beim Aendern von Layerdaten muss man derzeit
// noch selbst das Modify-Flag am Model setzen.

#define SDRLAYER_MAXCOUNT 255

class SVX_DLLPUBLIC SdrLayerAdmin
{
private:
    // allow SdrUndoNewLayer/SdrUndoDelLayer to call
    // InsertLayerFromUndoRedo to execute it's undo/redo
    friend class SdrUndoNewLayer;
    friend class SdrUndoDelLayer;

    void InsertLayerFromUndoRedo(SdrLayer* pLayer, sal_uInt32 nPos = 0xffffffff);

protected:
    typedef ::std::vector< SdrLayer* > SdrLayerContainerType;

    SdrLayerContainerType   maLayer;
    SdrLayerAdmin*          mpParent; // Der Admin der Seite kennt den Admin des Docs
    SdrModel&               mrModel; // zum Broadcasten
    String                  maControlLayerName;

protected:
    // Eine noch nicht verwendete LayerID raussuchen. Sind bereits alle
    // verbraucht, so gibt's 'ne 0. Wer sicher gehen will, muss vorher
    // GetLayerCount()<SDRLAYER_MAXCOUNT abfragen, denn sonst sind alle
    // vergeben.
    SdrLayerID           GetUniqueLayerID() const;
    void                 Broadcast() const;

public:
    SdrLayerAdmin(SdrModel& rSdrModel, SdrLayerAdmin* pNewParent = 0);
    ~SdrLayerAdmin();

    bool             operator==(const SdrLayerAdmin& rCmpLayerAdmin) const;
    bool             operator!=(const SdrLayerAdmin& rCmpLayerAdmin) const       { return !operator==(rCmpLayerAdmin); }

    SdrLayerAdmin* GetParent() const { return mpParent; }
    void SetParent(SdrLayerAdmin* pNewParent) { if(mpParent != pNewParent) mpParent = pNewParent; }

    SdrModel& GetSdrModel() const { return mrModel; }
    SdrLayer* RemoveLayer(sal_uInt32 nPos);

    // Alle Layer loeschen
    void               ClearLayer();

    // Neuer Layer wird angelegt und eingefuegt
    SdrLayer* NewLayer(const String& rName, sal_uInt32 nPos = 0xffffffff);
    void DeleteLayer(SdrLayer* pLayer);

    // Neuer Layer, Name wird aus der Resource geholt
    SdrLayer* NewStandardLayer(sal_uInt32 nPos = 0xffffffff);

    // Iterieren ueber alle Layer
    sal_uInt32 GetLayerCount() const { return maLayer.size(); }
    SdrLayer* GetLayer(sal_uInt32 i) const;

    sal_uInt32 GetLayerPos(SdrLayer* pLayer) const;
    SdrLayer* GetLayer(const String& rName, bool bInherited) const;
    SdrLayerID GetLayerID(const String& rName, bool bInherited) const;
    SdrLayer* GetLayerPerID(SdrLayerID nID) const;

    void SetControlLayerName(const String& rNewName) { maControlLayerName = rNewName; }
    const String& GetControlLayerName() const;
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
    Jeder LayerAdmin einer Seite hat einen Parent-LayerAdmin, nämlich den des
    Model. Das Model selbst hat keinen Parent.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDLAYER_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
