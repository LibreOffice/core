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

#ifndef _CTRLTOOL_HXX
#define _CTRLTOOL_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include "svtools/svtdllapi.h"
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/metric.hxx>

class ImplFontListNameInfo;
class OutputDevice;

/*************************************************************************

Beschreibung
============

class FontList

Diese Klasse verwaltet alle Fonts, die auf einem oder zwei Ausgabegeraeten
dargestellt werden koennen. Zusaetzlich bietet die Klasse Methoden an, um
aus Fett und Kursiv den StyleName zu generieren oder aus einem Stylename
die fehlenden Attribute. Zusaetzlich kann diese Klasse syntetisch nachgebildete
Fonts verarbeiten. Diese Klasse kann mit verschiedenen Standard-Controls und
Standard-Menus zusammenarbeiten.

Querverweise

class FontNameBox, class FontStyleBox, class FontSizeBox,
class FontNameMenu, class FontSizeMenu

--------------------------------------------------------------------------

FontList::FontList( OutputDevice* pDevice, OutputDevice* pDevice2 = NULL,
                    sal_Bool bAll = sal_True );

Konstruktor der Klasse FontList. Vom uebergebenen OutputDevice werden die
entsprechenden Fonts abgefragt. Das OutputDevice muss solange existieren,
wie auch die Klasse FontList existiert. Optional kann noch ein 2tes
Ausgabedevice uebergeben werden, damit man zum Beispiel die Fonts von
einem Drucker und dem Bildschirm zusammen in einer FontListe verwalten kann
und somit auch den FontListen und FontMenus die Fonts von beiden OutputDevices
zu uebergeben. Auch das pDevice2 muss solange existieren, wie die Klasse
FontList existiert.

Das OutputDevice, welches als erstes uebergeben wird, sollte das bevorzugte
sein. Dies sollte im normalfall der Drucker sein. Denn wenn 2 verschiede
Device-Schriften (eine fuer Drucker und eine fuer den Bildschirm) vorhanden
sind, wird die vom uebergebenen Device "pDevice" bevorzugt.

Mit dem dritten Parameter kann man angeben, ob nur skalierbare Schriften
abgefragt werden sollen oder alle. Wenn sal_True uebergeben wird, werden auch
Bitmap-Schriften mit abgefragt. Bei sal_False werden Vector-Schriften und
scalierbare Schriften abgefragt.

--------------------------------------------------------------------------

String FontList::GetStyleName( const FontInfo& rInfo ) const;

Diese Methode gibt den StyleName von einer FontInfo zurueck. Falls kein
StyleName gesetzt ist, wird aus den gesetzten Attributen ein entsprechender
Name generiert, der dem Anwender praesentiert werden kann.

--------------------------------------------------------------------------

OUString FontList::GetFontMapText( const FontInfo& rInfo ) const;

Diese Methode gibt einen Matchstring zurueck, der dem Anwender
anzeigen soll, welche Probleme es mit diesem Font geben kann.

--------------------------------------------------------------------------

FontInfo FontList::Get( const String& rName, const String& rStyleName ) const;

Diese Methode sucht aus dem uebergebenen Namen und dem uebergebenen StyleName
die entsprechende FontInfo-Struktur raus. Der Stylename kann in dieser
Methode auch ein syntetischer sein. In diesem Fall werden die entsprechenden
Werte in der FontInfo-Struktur entsprechend gesetzt. Wenn ein StyleName
uebergeben wird, kann jedoch eine FontInfo-Struktur ohne Stylename
zurueckgegeben werden. Um den StyleName dem Anwender zu repraesentieren,
muss GetStyleName() mit dieser FontInfo-Struktur aufgerufen werden.

Querverweise

FontList::GetStyleName()

--------------------------------------------------------------------------

FontInfo FontList::Get( const String& rName, FontWeight eWeight,
                        FontItalic eItalic ) const;

Diese Methode sucht aus dem uebergebenen Namen und den uebergebenen Styles
die entsprechende FontInfo-Struktur raus. Diese Methode kann auch eine
FontInfo-Struktur ohne Stylename zurueckgegeben. Um den StyleName dem
Anwender zu repraesentieren, muss GetStyleName() mit dieser FontInfo-Struktur
aufgerufen werden.

Querverweise

FontList::GetStyleName()

--------------------------------------------------------------------------

const sal_IntPtr* FontList::GetSizeAry( const FontInfo& rInfo ) const;

Diese Methode liefert zum uebergebenen Font die vorhandenen Groessen.
Falls es sich dabei um einen skalierbaren Font handelt, werden Standard-
Groessen zurueckgegeben. Das Array enthaelt die Hoehen des Fonts in 10tel
Point. Der letzte Wert des Array ist 0. Das Array, was zurueckgegeben wird,
wird von der FontList wieder zerstoert. Nach dem Aufruf der naechsten Methode
von der FontList, sollte deshalb das Array nicht mehr referenziert werden.

*************************************************************************/

// ------------
// - FontList -
// ------------

#define FONTLIST_FONTINFO_NOTFOUND  ((sal_uInt16)0xFFFF)

#define FONTLIST_FONTNAMETYPE_PRINTER           ((sal_uInt16)0x0001)
#define FONTLIST_FONTNAMETYPE_SCREEN            ((sal_uInt16)0x0002)

class SVT_DLLPUBLIC FontList
{
private:
    OUString                maMapBoth;
    OUString                maMapPrinterOnly;
    OUString                maMapScreenOnly;
    OUString                maMapSizeNotAvailable;
    OUString                maMapStyleNotAvailable;
    mutable OUString        maMapNotAvailable;
    OUString                maLight;
    OUString                maLightItalic;
    OUString                maNormal;
    OUString                maNormalItalic;
    OUString                maBold;
    OUString                maBoldItalic;
    OUString                maBlack;
    OUString                maBlackItalic;
    sal_IntPtr*             mpSizeAry;
    OutputDevice*           mpDev;
    OutputDevice*           mpDev2;
    boost::ptr_vector<ImplFontListNameInfo> maEntries;

    SVT_DLLPRIVATE ImplFontListNameInfo*    ImplFind( const OUString& rSearchName, sal_uLong* pIndex ) const;
    SVT_DLLPRIVATE ImplFontListNameInfo*    ImplFindByName( const OUString& rStr ) const;
    SVT_DLLPRIVATE void                 ImplInsertFonts( OutputDevice* pDev, sal_Bool bAll,
                                             sal_Bool bInsertData );

public:
                            FontList( OutputDevice* pDevice,
                                      OutputDevice* pDevice2 = NULL,
                                      sal_Bool bAll = sal_True );
                            ~FontList();

    FontList*               Clone() const;

    OutputDevice*           GetDevice() const { return mpDev; }
    OutputDevice*           GetDevice2() const { return mpDev2; }
    OUString                GetFontMapText( const FontInfo& rInfo ) const;

    const OUString&         GetNormalStr() const { return maNormal; }
    const OUString&         GetItalicStr() const { return maNormalItalic; }
    const OUString&         GetBoldStr() const { return maBold; }
    const OUString&         GetBoldItalicStr() const { return maBoldItalic; }
    const OUString&         GetStyleName( FontWeight eWeight, FontItalic eItalic ) const;
    OUString                GetStyleName( const FontInfo& rInfo ) const;

    FontInfo                Get( const OUString& rName,
                                 const OUString& rStyleName ) const;
    FontInfo                Get( const OUString& rName,
                                 FontWeight eWeight,
                                 FontItalic eItalic ) const;

    sal_Bool                IsAvailable( const OUString& rName ) const;
    sal_uInt16              GetFontNameCount() const
    {
        return (sal_uInt16)maEntries.size();
    }
    const FontInfo&         GetFontName( sal_uInt16 nFont ) const;
    sal_Handle              GetFirstFontInfo( const OUString& rName ) const;
    sal_Handle              GetNextFontInfo( sal_Handle hFontInfo ) const;
    const FontInfo&         GetFontInfo( sal_Handle hFontInfo ) const;

    const sal_IntPtr*       GetSizeAry( const FontInfo& rInfo ) const;
    static const sal_IntPtr* GetStdSizeAry();

private:
                            FontList( const FontList& );
    FontList&               operator =( const FontList& );
};


// -----------------
// - FontSizeNames -
// -----------------

class SVT_DLLPUBLIC FontSizeNames
{
private:
    const struct ImplFSNameItem*    mpArray;
    sal_uLong                   mnElem;

public:
                            FontSizeNames( LanguageType eLanguage /* = LANGUAGE_DONTKNOW */ );

    sal_uLong                   Count() const { return mnElem; }
    sal_Bool                    IsEmpty() const { return !mnElem; }

    long                    Name2Size( const OUString& ) const;
    OUString                Size2Name( long ) const;

    OUString                GetIndexName( sal_uLong nIndex ) const;
    long                    GetIndexSize( sal_uLong nIndex ) const;
};

#endif  // _CTRLTOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
