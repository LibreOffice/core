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

#ifndef _SWGIDS_HXX
#define _SWGIDS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

struct FileHeader {
    long   nSignature;				// Signatur: "SWG1"
    USHORT nVersion;				// Versionsnummer der Datei
    USHORT nFlags;					// Flags (s.u.)
    long   nFree1;					// 1. freier Textblock (meist 0)
    long   nDocInfo;				// Offset der statischen DocInfo
    sal_Char   cPasswd[ 16 ];		// Passwort, falls vorhanden
                                    // in SWGINF_EXTINFO:
//	BYTE   cGUIType;				// aktuell verwendetes GUI
//	BYTE   cCharSet;				// aktuell verwendeter Zeichensatz
};

#define	SWGF_NO_FRAMES	0x0001		// keine Layout-Frames einlesen
#define	SWGF_HAS_BLOCKS	0x0002		// Datei enthaelt Textbausteine
#define	SWGF_HAS_VERS	0x0004		// Datei enthaelt mehrere Versionen
#define	SWGF_HAS_PASSWD	0x0008		// Datei ist passwortgeschuetzt
#define SWGF_HAS_GRFLNK	0x0010		// Datei enthaelt Grafik-Links
#define SWGF_HAS_DDELNK	0x0020		// Datei enthaelt DDE-Links
#define SWGF_HAS_OLELNK	0x0040		// Datei enthaelt OLE-Links
#define	SWGF_PORT_GRAF	0x0080		// Grafiken sollen portabel sein
#define SWGF_HAS_PGNUMS 0x0100		// Datei hat Seitennummern
#define	SWGF_FIX_FMTS	0x4000		// Nicht alle Formate gespeichert
#define	SWGF_BAD_FILE	0x8000		// Fehler beim Schreiben

// Header-Record im Doc-Record

struct DocHeader {					// Dokument-Header:
    USHORT nFlags;					// Dokument-Flags
    USHORT nVersion;				// aktuelle Version
    BYTE cAttrTab [8];				// Start-IDs der Attribute
    BYTE cAttrSiz [8];				// Laengen der Attribut-Gruppen
    BYTE cCodeSet;					// Zeichensatz des Systems
    USHORT nNumRules;				// Anzahl Numerierungs-Regeln
    long nDocVersion;				// externe Versionsnummer
    BYTE cGUIType;					// Verwendetes System
    BYTE cReserved[ 3 ];			// reserviert
    long nDocInfoOffset;			// Offset der Dokument-Infos
    long nLayoutOffset;				// Offset der Layout-Frames
    long nDate, nTime;				// neu: Datum, Uhrzeit der Speicherung
};

enum DocFlags {						// Dokument-Flags:
    DOCF_NAMEDFMTS = 0x0001,		// hat Einzel-Formatvorlagen
    DOCF_FMTCOLLS  = 0x0002,		// hat Format-Collections
    DOCF_NUMRULES  = 0x0004,		// hat Numerierungs-Regeln
    DOCF_CONTENTS  = 0x0008,		// hat Inhalt
    DOCF_DOCINFO   = 0x0010			// hat Dokument-Info
};

enum AttrIdx { 						// Indexe von cAttrTab[]
    ATTR_CHR1 = 0,					// 1. Attribut RES_CHRATR
    ATTR_FONT1,						// 1. Attribut FONT_TXTATR
    ATTR_TEXT1,						// 1. Attribut ETC_TXTATR
    ATTR_PAR1,						// 1. Attribut RES_PARATR
    ATTR_ADD1,						// 1. Attribut RES_ADDATR
    ATTR_FRM1,						// 1. Attribut RES_FRMATR
    ATTR_GRF1						// 1. Attribut RES_GRFATR
};

#define	SWG_SIGN1		"SWG1"		// Alte Signatur vor 2.0
#define SWG_SIGNATURE	"SWG2"		// Datei-Signatur

                                    // Der Reader braucht diese IDs noch
#define	SWG_VERSION1	0x0007 		// 1. kompatible Version
#define	SWG_VER_LRSPACE	0x000A 		// Aenderung LRSpace und FirstLineOffset
#define	SWG_VER_NUM		0x000C 		// Numerierung, Flags in TxtNode
#define	SWG_VER_FIELD	0x000E 		// Feldnummern mit Offset=0
#define SWG_VER_FRMSIZE	0x000F 		// Hint FrmSize mit 4 Byte-Longs
#define	SWG_VER_OLDHDR	0x0010		// alter Dateikopf
#define	SWG_VER_FMTCOL	0x0012		// neue Form des FmtCol-Hints
#define	SWG_VER_KERNING	0x0013		// neue Form des Kerning-Hints
#define	SWG_VER_FRAMES	0x0015		// Tabellenformat als Index, Layoutframes
#define	SWG_VER_ST		0x0016		// STs Wunsch nach besserem JobSetup
#define	SWG_VER_LCLFMT	0x0017		// Autoformate local im Content/Textblock
#define	SWG_VER_HIDPARA	0x0018		// neues Feld HiddenPara, Wert bei UserFld
#define	SWG_VER_BOOKMAC	0x0019		// Makros an Bookmarks
#define	SWG_VER_STAMP	0x001A		// Time Stamp im Doc-Header
#define	SWG_VER_FMTNAME	0x001B		// alle Formate mit Namen
#define	SWG_VER_BETA3	0x001C		// offizielle Betaversion 3
#define	SWG_VER_PORTGRF	0x001D		// Portable Grafiken
#define	SWG_VER_PRODUCT	0x0100		// Produktversion
#define SWG_VER_COLLREF 0x0101		// Collections. Referenzen auf ben. Fmts
#define	SWG_VER_COMPAT	0x0200		// Besser rueckwaertskompatible Version
#define	SWG_VER_POOLIDS	0x0201		// Neues PoolId-Handling
#define	SWG_VER_DOCINFO	0x0202		// eigene SwSwgInfo-Klasse
#define	SWG_VER_NPOOLIDS 0x0203		// neue Pool-Id's, noch abs. lesen/speichern
#define	SWG_VER_FRAMES3	0x0204		// Frame-Infos Version 3
#define	SWG_VER_POOLID3	0x0205		// Pool-Ids, der dritte Versuch!
#define	SWG_VER_TOXDATA	0x0206		// SWG_DATA-Kapselung im TOX-Record
#define	SWG_VER_GETEXP 	0x0207		// neuer Parameter in GetExp-Feld
#define SWG_VER_TBLSIZE 0x0208		// FrmSize von Tbl von LONG_MAX nach USHRT_MAX
                                    // FrmSize der Boxen entsprechend anpassen.
#define SWG_VER_NEWALIGN 0x0209		// Alignment wurde Layoutseitig verandert.
                                    // SwFmtBox alter Versionen bekommt eine
                                    // minimale Distance verpasst.
#define	SWG_VER_NEWJOB	0x020A		// SWG_NEWJOBSETUP
#define	SWG_VER_POOLID2	0x020B		// Bitverschiebungen im Pool-ID

#define SWG_VERSION     0x020B      // aktuelle Version

                                    // Kennungen fuer Dokument-Info:
#define	SWGINF_END		0x00		// Ende der Informationen
#define	SWGINF_DBNAME	0x02		// Datenbankname
#define	SWGINF_AUTHOR	0x03		// Autor
#define SWGINF_DOCSTAT  0x04		// Statistiken
#define	SWGINF_SAVEINFO	0x05		// Save-Infoblock
#define	SWGINF_EXTINFO	0x06		// Extended Infos
#define	SWGINF_LAYOUTPR	0x07		// Drucker fuer Layout
                                    // Im dynamischen Teil am Dateiende:
#define	SWGINF_TEMPLATE 0x08		// Template-Infos
#define	SWGINF_ADDRINFO	0x09		// Adressaten-Eintrag

                                            // Indexwerte fuer Formate:
#define	IDX_NO_VALUE	(USHORT) 0xFFFF		// Indexwert nicht belegt
#define	IDX_DFLT_VALUE	(USHORT) 0xFFFE		// Index zeigt auf Defaultwert
#define	IDX_COLUMN		(USHORT) 0xFFFD		// Default-Spalten-Frameformat
#define	IDX_EMPTYPAGE	(USHORT) 0xFFFC		// Default-Format leere Seite
#define	IDX_SPECIAL		(USHORT) 0xFFF0		// Beginn Spezialformate
#define	IDX_TYPEMASK	(USHORT) 0xE000		// Typmaske fuer Formate
#define	IDX_AUTOFMT		(USHORT) 0x0000		// Autoformat
#define	IDX_FLYFMT		(USHORT) 0x2000		// FlyFormat
#define	IDX_NAMEDFMT	(USHORT) 0x4000		// benanntes Format
#define	IDX_FLOATFMT	(USHORT) 0x6000		// freies Header/Footer-Format
#define	IDX_COLLECTION	(USHORT) 0x8000		// Format-Collection
#define	IDX_MASTERFMT	(USHORT) 0xA000		// Seitendeskriptor-Masterfmt
#define	IDX_LEFTFMT		(USHORT) 0xC000		// Seitendeskriptor-Leftfmt

#define	SWG_GRFATR1		0xF0		// 1. RES_GRFATR  (max 16)
#define	SWG_FRMATR1		0xD0		// 1. RES_FRMATR  (max 32)
#define	SWG_ADDATR1		0xC0		// 1. RES_ADDATR  (max 16)
#define	SWG_PARATR1		0xA0		// 1. RES_PARATR  (max 32)
#define	SWG_TEXTATR1	0x80		// 1. ETC_TXTATR  (max 32)
#define SWG_FONTATR1	0x60		// 1. FONT_TXTATR (max 32)
#define	SWG_CHRATR1		0x40		// 1. RES_CHRATR  (max 32)

#ifndef SW3_NEW_FORMAT

#define	SWG_MAXGLOBAL	0x3F		// Maximalwert fuer globale Records

#define	SWG_STRINGPOOL	0x39		// String-Pool
#define SWG_SDRFMT		0x38		// SdrObject-Frameformat
#define	SWG_OLEVISRECT	0x37		// OLE2 VisRectangle
                                    // Alte SWG2-IDs:
#define	SWG_NEWJOBSETUP	0x36		// neue JobSetup-Daten mit Stream-Operator
#define	SWG_DYNDOCINFO	0x35		// DocInfo am Dateiende
#define	SWG_DATA		0x34		// Wrapper fuer binaere Daten
#define	SWG_FIELDTYPES	0x33		// Nicht-Systemfeldtypen
#define	SWG_TEXTBLOCK	0x32		// Textblock
#define	SWG_NAMEDFMTS	0x31		// benannte Formate
#define	SWG_DOCUMENT	0x30		// Dokument-Beginn
#define	SWG_FRAMES		0x2F		// Layout-Frames
#define	SWG_MACROTBL	0x2E		// Makro-Tabelle

#define SWG_TOXGROUP    0x2D        // Verzeichnis-Gruppe
#define SWG_TOXDESC     0x2C        // Verzeichnis-Descriptor

#define SWG_NUMRULE     0x2B        // Numerierungs-Regelwerk
#define SWG_NUMFMT      0x2A        // Numerierungs-Format
#define	SWG_NUMRULES	0x29		// Numerierungs-Regeln im Text
#define	SWG_OUTLINE		0x28		// Outline-Numerierung

#define	SWG_DOCINFO		0x27		// Dokument-Info

#define	SWG_TABLEBOX	0x26		// Tabellen-Zelle
#define	SWG_TABLELINE	0x25		// Tabellen-Zeile
#define	SWG_TABLE		0x24		// Tabelle

#define	SWG_BOTTOM		0x23		// unterer Rand
#define	SWG_RIGHT		0x22		// rechter Rand
#define	SWG_LEFT		0x21		// linker Rand
#define	SWG_TOP			0x20		// oberer Rand
#define	SWG_BOOKMARK	0x1F		// Bookmark
#define	SWG_OLE			0x1E		// OLE-Objekt
#define	SWG_GRAPHIC		0x1D		// Grafik-Objekt
#define	SWG_FMTHINTS	0x1C		// Liste von Formathints
#define	SWG_TEXTHINTS	0x1B		// Liste von Texthints
#define	SWG_TEXT		0x1A		// Textblock
#define	SWG_LAYOUT		0x19		// Layoutbeschreibung
#define	SWG_FOOTINFO	0x18		// Layout-Fussnoten-Info
#define	SWG_JOBSETUP	0x17		// JobSetup-Daten
#define	SWG_TXTCOLL   	0x16		// Text-Format-Collection
#define	SWG_COLLNEXT	0x15		// TextCollection-Folgeliste (alt)
#define	SWG_COLLDERIVED	0x14		// TextCollection-Abhaengigkeiten (alt)
#define	SWG_COLLECTIONS	0x13		// Tabelle der Formate
#define	SWG_PAGEDESC	0x12		// Layout-Seitenbeschreibung

#define	SWG_OLENODE		0x11		// OLE-Node
#define	SWG_GRFNODE		0x10		// Grafik-Node
#define SWG_TEXTNODE	0x0F		// Textnode

#define	SWG_FREEFMT		0x0E		// Frameformat ohne Doc-Anbindung
#define	SWG_GRFFMT		0x0D		// Grafik-Formatbeschreibung
#define	SWG_FLYFMT		0x0C		// FlyFrame-Formatbeschreibung
#define	SWG_FRAMEFMT	0x0B		// Frame-Formatbeschreibung
#define	SWG_CHARFMT		0x0A		// Zeichen-Formatbeschreibung
#define	SWG_PARAFMT		0x09		// Absatz-Formatbeschreibung
#define	SWG_MASTERFMT	0x08		// Layout-Masterformat
#define	SWG_LEFTFMT		0x07		// Layout-Format linke Seite

#define	SWG_AUTOFMTS	0x06		// Auto-Formate
#define	SWG_DFLTFMTS	0x05		// Default-Formate
#define	SWG_CONTENTS	0x04		// Textteil
#define	SWG_LAYOUTS		0x03		// Layout-Teil
#define	SWG_FLYFRAMES	0x02		// FlyFrames
#define SWG_COMMENT		0x01		// Kommentare

#define SWG_EOF			0x00		// EOF-Markierung

// Der SWG_COMMENT-Record wird ab sofort erweitert, um rueckwaertskompatibel
// neue Records einfuehren zu koennen. Da zur Zeit Comment-Records mit Strings
// (SW/G-Version) existieren, ist das erste Zeichen 00H; danach folgt der
// erweiterte Record-Typ, der unten definiert ist.

#define	SWG_XFTNCOLL	0x01		// Collection fuer Fussnoten

#endif

} //namespace binfilter
#endif
