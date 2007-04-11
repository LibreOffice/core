/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdlayer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:21:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDLAYER_HXX
#define _SVDLAYER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _SVDSOB_HXX //autogen
#include <svx/svdsob.hxx>
#endif

#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx> // fuer typedef SdrLayerID
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrModel;

class SVX_DLLPUBLIC SdrLayer
{
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
    bool      operator==(const SdrLayer& rCmpLayer) const;
    bool      operator!=(const SdrLayer& rCmpLayer) const { return !operator==(rCmpLayer); }
    void          SetName(const String& rNewName);
    const String& GetName() const                             { return aName; }
    SdrLayerID    GetID() const                               { return nID; }
    void          SetModel(SdrModel* pNewModel)               { pModel=pNewModel; }
    SdrModel*     GetModel() const                            { return pModel; }
    // Einem SdrLayer kann man sagen dass er ein (der) Standardlayer sein soll.
    // Es wird dann laenderspeziefisch der passende Name gesetzt. SetName()
    // setzt das Flag "StandardLayer" ggf. zurueck auf "Userdefined".
    void          SetStandardLayer(FASTBOOL bStd=TRUE);
    FASTBOOL      IsStandardLayer() const                     { return nType==1; }
};

// Beim Aendern von Layerdaten muss man derzeit
// noch selbst das Modify-Flag am Model setzen.
#define SDRLAYER_MAXCOUNT 255
class SVX_DLLPUBLIC SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;

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
    void                 Broadcast() const;
public:
    SdrLayerAdmin(SdrLayerAdmin* pNewParent=NULL);
    SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin);
    ~SdrLayerAdmin();
    const SdrLayerAdmin& operator=(const SdrLayerAdmin& rSrcLayerAdmin);
    bool             operator==(const SdrLayerAdmin& rCmpLayerAdmin) const;
    bool             operator!=(const SdrLayerAdmin& rCmpLayerAdmin) const       { return !operator==(rCmpLayerAdmin); }
    SdrLayerAdmin*       GetParent() const                                           { return pParent; }
    void                 SetParent(SdrLayerAdmin* pNewParent)                        { pParent=pNewParent; }
    void                 SetModel(SdrModel* pNewModel);
    SdrModel*            GetModel() const                                            { return pModel; }
    void                 InsertLayer(SdrLayer* pLayer, USHORT nPos=0xFFFF)           { aLayer.Insert(pLayer,nPos); pLayer->SetModel(pModel); Broadcast(); }
    SdrLayer*            RemoveLayer(USHORT nPos);
    // Alle Layer loeschen
    void               ClearLayer();
    // Neuer Layer wird angelegt und eingefuegt
    SdrLayer*          NewLayer(const String& rName, USHORT nPos=0xFFFF);
    void               DeleteLayer(SdrLayer* pLayer)                                 { aLayer.Remove(pLayer); delete pLayer; Broadcast(); }
    void               MoveLayer(SdrLayer* pLayer, USHORT nNewPos=0xFFFF);
    SdrLayer*          MoveLayer(USHORT nPos, USHORT nNewPos);
    // Neuer Layer, Name wird aus der Resource geholt
    SdrLayer*          NewStandardLayer(USHORT nPos=0xFFFF);

    // Iterieren ueber alle Layer
    USHORT             GetLayerCount() const                                         { return USHORT(aLayer.Count()); }
    SdrLayer*          GetLayer(USHORT i)                                            { return (SdrLayer*)(aLayer.GetObject(i)); }
    const SdrLayer*    GetLayer(USHORT i) const                                      { return (SdrLayer*)(aLayer.GetObject(i)); }

    USHORT             GetLayerPos(SdrLayer* pLayer) const;

    SdrLayer*          GetLayer(const String& rName, FASTBOOL bInherited)            { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayer(rName,bInherited)); }
    const SdrLayer*    GetLayer(const String& rName, FASTBOOL bInherited) const;
          SdrLayerID   GetLayerID(const String& rName, FASTBOOL bInherited) const;
          SdrLayer*    GetLayerPerID(USHORT nID)                                     { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayerPerID(nID)); }
    const SdrLayer*    GetLayerPerID(USHORT nID) const;

    void               SetControlLayerName(const String& rNewName) { aControlLayerName=rNewName; }
    const String&      GetControlLayerName() const                 { return aControlLayerName; }
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

#endif //_SVDLAYER_HXX

