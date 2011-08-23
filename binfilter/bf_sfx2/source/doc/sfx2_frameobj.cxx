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

#include <sot/clsids.hxx>
#include <bf_so3/ipwin.hxx>


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "frameobj.hxx"

#include "frmdescr.hxx"

#include <appdata.hxx>

#include "app.hxx"

#include "objsh.hxx"

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {







/*N*/ struct SfxFrameObject_Impl
/*	[Beschreibung]

    Impl-Struktur des SfxFrameObjects.
*/
/*N*/ {
/*N*/ 	SfxFrameDescriptor*		pFrmDescr;
/*N*/ 	Timer* 					pNewObjectTimer;
/*N*/ 	SfxObjectShellLock		xDoc;
/*N*/ 
/*N*/ 				SfxFrameObject_Impl()
/*N*/ 					: pFrmDescr( new SfxFrameDescriptor( NULL ) )
/*N*/ 					, pNewObjectTimer( NULL )
/*N*/ 				{
/*N*/ 				}
/*N*/ 
/*N*/ 				~SfxFrameObject_Impl()
/*N*/ 				{
/*N*/ 					// Der Descriptor geh"ort dem Object, andere erhalten
/*N*/ 					// davon immer nur Clones
/*N*/ 					delete pFrmDescr;
/*N*/ 				}
/*N*/ };





//=========================================================================
/*	[Beschreibung]

    Das Environment wird im <SfxFrameObject::InPlaceActivate()> angelegt.
    Durch die Verbindung mit dem Container Environment kann "uber die
    UI-Tools und Fenster verhandelt werden.

    [Querverweise]

    <SvInPlaceEnvironment>, <SvContainerEnvironment>
*/

//=========================================================================
/*	[Beschreibung]

    Die angelegten Fenster werden zerst"ort.
*/


//=========================================================================
//============== SfxFrameObject ===========================================
//=========================================================================
/*N*/ SO2_IMPL_BASIC_CLASS1_DLL( SfxFrameObject, SvFactory, SvInPlaceObject,
/*N*/ 			  SvGlobalName( BF_SO3_IFRAME_CLASSID ) )

//=========================================================================
/*?*/ SfxFrameObject::SfxFrameObject()
/*?*/ 	: pImpl( new SfxFrameObject_Impl )
/*	[Beschreibung]

    Konstruktor der Klasse SfxFrameObject. Die Verbliste und das
    Format werden global initialisiert.
*/
/*?*/ {
/*?*/ }

//=========================================================================
/*?*/ SfxFrameObject::~SfxFrameObject()
/*	[Beschreibung]

    Destruktor der Klasse SfxFrameObject.
*/
/*?*/ {
/*?*/ 	delete pImpl;
/*?*/ }


//=========================================================================
/*	[Beschreibung]

    Wenn die Daten sich "andern, muss das Modify-Flag gesetzt werden.
    Ausserdem m"ussen alle angemeldeten Advises benachrichtigt werden.
    In Abh"angigkeit des Parameters wird angezeigt, dass sich auch
    die View und das Mtf ge"andert hat.
*/

//=========================================================================
/*	[Beschreibung]

    Mit dieser Methode werden Informationen "uber das Objekt angefordert.
    Wird das Objekt gesichert, dann werden diese Informationen in den
    Storage geschrieben.

    [Anmerkung]
*/

//=========================================================================
/*	[Beschreibung]

    Die Verbindung zum Client wird ge"offnet oder geschlossen.

    [Querverweise]

    <SvEmbeddedObject::Open()>
*/

//=========================================================================
/*	[Beschreibung]

    Das Objekt wird InPlace aktiviert oder deaktiviert.

    [Querverweise]

    <SvInPlaceObject::InPlaceActivate()>
*/

//=========================================================================
/*	[Beschreibung]

    Es wird Versucht ein Applet zu starten. Es gibt nur die M"oglichkeit
    InPlace zu aktivieren. Deshalb sind auch nur die Verben gestattet,
    die dies ausl"osen.

    [R"uckgabewert]

    BOOL			TRUE, es wurde InPlace aktiviert.
                    FALSE, es wurde nicht InPLace aktiviert.

    [Querverweise]

    <SvPseudoObject::Verb>
*/

//=========================================================================
/*	[Beschreibung]

    Der sichtbare Ausschnitt beginnt immer an der Position (0, 0).
*/


//=========================================================================
// aus PlugIn

//=========================================================================
/*	[Beschreibung]

    Ein Ausgabe ist nicht m"oglich und auch nicht n"otig.
    und als Unterschrift der URL ausgegeben,

    [Querverweise]

    <SvEmbeddedObject::Draw>
*/

//=========================================================================
/*	[Beschreibung]

    Nach dem Konstruktor muss diese Methode oder Load gerufen werden,
    um das Objekt zu initialisieren.
    <SvPersist::InitNew>

    [R"uckgabewert]

    BOOL			TRUE, Initialisierung hat geklappt.
                    FALSE, Initialisierung hat nicht geklappt, das Objekt
                    muss sofort freigegeben werden.

    [Querverweise]

*/

//=========================================================================
/*N*/ static const char * pDocName = "FloatingFrame";
#define nVersion 3

/*N*/ BOOL SfxFrameObject::Load
/*N*/ (
/*N*/ 	SvStorage * pStor	/* Storage aus dem das Objekt geladen wird. */
/*N*/ )
/*	[Beschreibung]

    Nach dem Konstruktor muss diese Methode oder InitNew gerufen werden,
    um das Objekt zu initialisieren.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geladen.
                    FALSE, das Objekt wurde nicht geladen, es
                    muss sofort freigegeben werden.

    [Querverweise]

    <SvPersist::Load>
*/
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }

//=========================================================================
/*N*/ BOOL SfxFrameObject::Save()
/*	[Beschreibung]

    Der Inhalt des Objektes wird in den, in <SfxFrameObject::InitNew>
    oder <SfxFrameObject::Load> "ubergebenen Storage, geschrieben.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geschreiben.
                    FALSE, das Objekt wurde nicht geschrieben. Es muss
                    die in der Klasse <SvPersist> beschrieben
                    Fehlerbehandlung erfolgen.

    [Querverweise]

    <SvPersist::Save>
*/
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }

//=========================================================================
/*N*/ BOOL SfxFrameObject::SaveAs
/*N*/ (
/*N*/ 	SvStorage *pStor	/* Storage, in den der Inhalt des Objekte
                           geschrieben wird */
/*N*/ )
/*	[Beschreibung]

    Der Inhalt des Objektes wird in pStor geschrieben.

    [Anmerkung]

    Der Storage wird nicht behalten.

    [R"uckgabewert]

    BOOL			TRUE, das Objekt wurde geschreiben.
                    FALSE, das Objekt wurde nicht geschrieben. Es muss
                    die in der Klasse <SvPersist> beschrieben
                    Fehlerbehandlung erfolgen.

    [Querverweise]

    <SvPersist::SaveAs>
*/
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }

//=========================================================================
/*	[Beschreibung]

    Ab diesen Zeitpunkt, bis zum <SfxFrameObject::SaveCompleted>,
    darf auf den Storage im Objekt nicht zugegriffen werden.

    [Querverweise]

    <SvPersist::HandsOff>
*/

//=========================================================================
/*N*/ BOOL SfxFrameObject::SaveCompleted
/*N*/ (
/*N*/ 	SvStorage * pStor	/* Storage auf dem das Objekt arbeitet. Der kann
                           auch NULL sein. Dies Bedeutet, es wird auf
                           dem alten Storage weiter gearbeitet */
/*N*/ )
/*	[Beschreibung]

    Nach dem Aufruf dieser Methode ist das Verhalten des Objektes
    wieder definiert.

    [R"uckgabewert]

    BOOL			TRUE, es kann auf dem neuen Storage gearbeitet werden.
                    FALSE, es kann nicht auf dem neuen Storage gearbeitet
                    werden

    [Querverweise]

    <SvPersist::SaveCompleted>
*/
/*N*/ {
/*N*/ 	return SvInPlaceObject::SaveCompleted( pStor );
/*N*/ }

//=========================================================================
/*	[Beschreibung]

    Da ein Applet immer aktiviert wird, es ein Link ist und er
    keine UI-Tools braucht, muss dieser Status zur"uckgegeben werden.

    [R"uckgabewert]

    ULONG           Es wird immer  SVOBJ_MISCSTATUS_LINK,
                    SVOBJ_MISCSTATUS_INSIDEOUT und
                    SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE zur"uckgegeben

    [Querverweise]

    <SvPseudoObject::GetMiscStatus()>
*/

//=========================================================================
/*	[Beschreibung]

    Dieser Typ von Objekt ist immer ein Link.

    [R"uckgabewert]

    BOOL            Immer TRUE.

    [Querverweise]

    <SvPseudoObject::IsLink()>
*/

/*?*/ void SfxFrameObject::SetFrameDescriptor( const SfxFrameDescriptor *pD )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

/*?*/ const SfxFrameDescriptor* SfxFrameObject::GetFrameDescriptor() const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*?*/ }



/*N*/ SfxFrameObjectFactoryPtr* SfxFrameObject::GetFactoryPtr()
/*N*/ {
/*N*/ 	return SFX_APP()->Get_Impl()->pSfxFrameObjectFactoryPtr;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
