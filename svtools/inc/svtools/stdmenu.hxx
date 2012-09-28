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

#ifndef _STDMENU_HXX
#define _STDMENU_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <vcl/menu.hxx>

class FontList;
class FontInfo;

/*************************************************************************

Beschreibung
============

class FontNameMenu

Beschreibung

Erlaubt die Auswahl von Fonts. Das Menu wird ueber Fill mit den FontNamen
gefuellt. Fill sortiert automatisch die FontNamen (inkl. aller Umlaute und
sprachabhaengig). Mit SetCurName()/GetCurName() kann der aktuelle Fontname
gesetzt/abgefragt werden. Wenn SetCurName() mit einem leeren String
aufgerufen wird, wird kein Eintrag als aktueller angezeigt (fuer DontKnow).
Vor dem Selectaufruf wird der ausgewaehlte Name automatisch als aktueller
gesetzt und wuerde beim naechsten Aufruf auch als aktueller Name angezeigt
werden. Deshalb sollte vor PopupMenu::Execute() gegebenenfalls mit
SetCurName() der aktuelle Fontname gesetzt werden.

Da die Id's und der interne Aufbau des Menus nicht bekannt ist, muss ein
Select-Handler gesetzt werden, um die Auswahl eines Namens mitzubekommen.

In dieses Menu koennen keine weiteren Items eingefuegt werden.

Spaeter soll auch das Menu die gleichen Bitmaps anzeigen, wie die
FontNameBox. Auf den Systemen, wo Menues nicht automatisch scrollen,
wird spaeter wohl ein A-Z Menu ziwschengeschaltet. Da ein Menu bei vielen
installierten Fonts bisher schon immer lange gebraucht hat, sollte dieses
Menu schon jetzt nur einmal erzeugt werden (da sonst das Kontextmenu bis
zu 10-Sekunden fuer die Erzeugung brauchen koennte).

Querverweise

FontList; FontSizeMenu; FontNameBox

--------------------------------------------------------------------------

class FontSizeMenu

Beschreibung

Erlaubt die Auswahl von Fontgroessen. Ueber Fill wird das FontSizeMenu
gefuellt und ueber GetCurHeight() kann die ausgewaehlte Fontgroesse
abgefragt werden. Mit SetCurHeight()/GetCurHeight() kann die aktuelle
Fontgroesse gesetzt/abgefragt werden. Wenn SetCurHeight() mit 0 aufgerufen
wird, wird kein Eintrag als aktueller angezeigt (fuer DontKnow). Vor dem
Selectaufruf wird die ausgewaehlte Groesse automatisch als aktuelle gesetzt
und wuerde beim naechsten Aufruf auch als aktuelle Groesse angezeigt werden.
Deshalb sollte vor PopupMenu::Execute() gegebenenfalls mit SetCurHeight()
die aktuelle Groesse gesetzt werden. Da die Groessen vom ausgewaehlten Font
abhaengen, sollte nach einer Aenderung des Fontnamen das Menu mit Fill mit
den Groessen des Fonts neu gefuellt werden.

Da die Id's und der interne Aufbau des Menus nicht bekannt ist, muss ein
Select-Handler gesetzt werden, um die Auswahl einer Groesse mitzubekommen.

Alle Groessen werden in 10tel Point angegeben.

In dieses Menu koennen keine weiteren Items eingefuegt werden.

Spaeter soll das Menu je nach System die Groessen anders darstelllen. Zum
Beispiel koennte der Mac spaeter vielleicht einmal die Groessen als Outline
darstellen, die als Bitmap-Fonts vorhanden sind.

Querverweise

FontList; FontNameMenu; FontSizeBox

*************************************************************************/

// ----------------
// - FontNameMenu -
// ----------------

class SVT_DLLPUBLIC FontNameMenu : public PopupMenu
{
private:
    OUString        maCurName;
    Link            maSelectHdl;
    Link            maHighlightHdl;

public:
                    FontNameMenu();
    virtual         ~FontNameMenu();

    virtual void    Select();
    virtual void    Highlight();

    void            Fill( const FontList* pList );

    void            SetCurName( const OUString& rName );
    const OUString& GetCurName() const { return maCurName; }

    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
    void            SetHighlightHdl( const Link& rLink ) { maHighlightHdl = rLink; }
    const Link&     GetHighlightHdl() const { return maHighlightHdl; }
};

// ----------------
// - FontSizeMenu -
// ----------------

class SVT_DLLPUBLIC FontSizeMenu : public PopupMenu
{
private:
    long*           mpHeightAry;
    long            mnCurHeight;
    Link            maSelectHdl;
    Link            maHighlightHdl;

public:
                    FontSizeMenu();
                    ~FontSizeMenu();

    virtual void    Select();
    virtual void    Highlight();

    void            Fill( const FontInfo& rInfo, const FontList* pList );

    void            SetCurHeight( long nHeight );
    long            GetCurHeight() const { return mnCurHeight; }

    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
    void            SetHighlightHdl( const Link& rLink ) { maHighlightHdl = rLink; }
    const Link&     GetHighlightHdl() const { return maHighlightHdl; }
};

#endif  // _STDMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
