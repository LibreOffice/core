/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVDLAYER_HXX
#define _SVDLAYER_HXX

#include <tools/string.hxx>

#include <tools/stream.hxx>

#include <bf_svx/svdsob.hxx>

#include <bf_svx/svdtypes.hxx> // fuer typedef SdrLayerID
namespace binfilter {

class SdrModel;
class SdrLayerSet;

class SdrLayer {
friend class SdrLayerSet;
friend class SdrLayerAdmin;
protected:
    String     aName;
    SdrModel*  pModel; // zum Broadcasten
    UINT16     nType;  // 0=Userdefined,1=Standardlayer
    SdrLayerID nID;
protected:
    SdrLayer(SdrLayerID nNewID, const String& rNewName)       { nID=nNewID; aName=rNewName; nType=0; pModel=NULL; }
    void SetID(SdrLayerID nNewID)                             { nID=nNewID; }
public:
    SdrLayer(): pModel(NULL),nType(0),nID(0)                  {}
    void          SetName(const String& rNewName);
    const String& GetName() const                             { return aName; }
    SdrLayerID    GetID() const                               { return nID; }
    void          SetModel(SdrModel* pNewModel)               { pModel=pNewModel; }
    SdrModel*     GetModel() const                            { return pModel; }
    // Einem SdrLayer kann man sagen dass er ein (der) Standardlayer sein soll.
    // Es wird dann laenderspeziefisch der passende Name gesetzt. SetName()
    // setzt das Flag "StandardLayer" ggf. zurueck auf "Userdefined".
    FASTBOOL      IsStandardLayer() const                     { return nType==1; }
    friend SvStream& operator>>(SvStream& rIn, SdrLayer& rLayer);
    friend SvStream& operator<<(SvStream& rOut, const SdrLayer& rLayer);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Der Layerset merkt sich eine Menge von LayerID's.
// Wird die ID eines zum Set gehoerigen Layer geaendert, so gehoert
// dieser damit nichtmehr zum Set ...
// Beim Einblenden eines Layerset in einer View werden alle Member-
// Layer sichtbar und alle Excluded-Layer unsichtbat geschaltet.
// alle anderen Layer bleiben unberuehrt.
class SdrLayerSet {
friend class SdrLayerAdmin;
friend class SdrView;
protected:
    //SdrLayerAdmin& rAd; // Admin, um Layernamen herauszufinden, ...
    String    aName;
    SetOfByte aMember;
    SetOfByte aExclude;
    SdrModel* pModel; // zum Broadcasten
protected:
    // Broadcasting ueber's Model und setzen des Modified-Flags
public:
    SdrLayerSet(): pModel(NULL) {}
    SdrLayerSet(const String& rNewName): aName(rNewName), pModel(NULL) {}
    void            SetModel(SdrModel* pNewModel)          { pModel=pNewModel; }
//    void            AddAll()                               { aMember.SetAll(); }
//    void            ExcludeAll()                           { aExclude.SetAll(); }
    friend SvStream& operator>>(SvStream& rIn, SdrLayerSet& rSet);
    friend SvStream& operator<<(SvStream& rOut, const SdrLayerSet& rSet);
};

// Beim Aendern von Layerdaten muss man derzeit
// noch selbst das Modify-Flag am Model setzen.
#define SDRLAYER_MAXCOUNT 255
class SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;
//friend class MyScr; // debug
protected:
    Container      aLayer;
    Container      aLSets;
    SdrLayerAdmin* pParent; // Der Admin der Seite kennt den Admin des Docs
    SdrModel*      pModel; // zum Broadcasten
    String         aControlLayerName;
protected:
    // Eine noch nicht verwendete LayerID raussuchen. Sind bereits alle
    // verbraucht, so gibt's 'ne 0. Wer sicher gehen will, muss vorher
    // GetLayerCount()<SDRLAYER_MAXCOUNT abfragen, denn sonst sind alle
    // vergeben.
    SdrLayerID           GetUniqueLayerID() const;
    // Broadcasting ueber's Model und setzen des Modified-Flags
    void                 Broadcast(FASTBOOL bLayerSet) const;
public:
    SdrLayerAdmin(SdrLayerAdmin* pNewParent=NULL);
    SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin);
    ~SdrLayerAdmin();
    SdrLayerAdmin*       GetParent() const                                           { return pParent; }
    void                 SetParent(SdrLayerAdmin* pNewParent)                        { pParent=pNewParent; }
    void                 SetModel(SdrModel* pNewModel);
    SdrModel*            GetModel() const                                            { return pModel; }
    void                 InsertLayer(SdrLayer* pLayer, USHORT nPos=0xFFFF)           { aLayer.Insert(pLayer,nPos); pLayer->SetModel(pModel); Broadcast(FALSE); }
    // Alle Layer loeschen
    void               ClearLayer();
    // Neuer Layer wird angelegt und eingefuegt
    SdrLayer*          NewLayer(const String& rName, USHORT nPos=0xFFFF);
    void               DeleteLayer(SdrLayer* pLayer)                                 { aLayer.Remove(pLayer); delete pLayer; Broadcast(FALSE); }
    // Neuer Layer, Name wird aus der Resource geholt

    // Iterieren ueber alle Layer
    USHORT             GetLayerCount() const                                         { return USHORT(aLayer.Count()); }
    SdrLayer*          GetLayer(USHORT i)                                            { return (SdrLayer*)(aLayer.GetObject(i)); }
    const SdrLayer*    GetLayer(USHORT i) const                                      { return (SdrLayer*)(aLayer.GetObject(i)); }


    SdrLayer*          GetLayer(const String& rName, FASTBOOL bInherited)            { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayer(rName,bInherited)); }
    const SdrLayer*    GetLayer(const String& rName, FASTBOOL bInherited) const;
          SdrLayerID   GetLayerID(const String& rName, FASTBOOL bInherited) const;
          SdrLayer*    GetLayerPerID(USHORT nID)                                     { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayerPerID(nID)); }
    const SdrLayer*    GetLayerPerID(USHORT nID) const;

    void               InsertLayerSet(SdrLayerSet* pSet, USHORT nPos=0xFFFF)         { aLSets.Insert(pSet,nPos); pSet->SetModel(pModel); Broadcast(TRUE); }
    // Alle LayerSets loeschen
    void               ClearLayerSets();
    // Neuer Layerset wird angelegt und eingefuegt
    void               DeleteLayerSet(SdrLayerSet* pSet)                             { aLSets.Remove(pSet); delete pSet; Broadcast(TRUE); }
    // Iterieren ueber alle LayerSets
    USHORT             GetLayerSetCount() const                                      { return USHORT(aLSets.Count()); }
    SdrLayerSet*       GetLayerSet(USHORT i)                                         { return (SdrLayerSet*)(aLSets.GetObject(i)); }
    const SdrLayerSet* GetLayerSet(USHORT i) const                                   { return (SdrLayerSet*)(aLSets.GetObject(i)); }


    void     	       SetControlLayerName(const String& rNewName) { aControlLayerName=rNewName; }
    const String& 	   GetControlLayerName() const                 { return aControlLayerName; }
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

}//end of namespace binfilter
#endif //_SVDLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
