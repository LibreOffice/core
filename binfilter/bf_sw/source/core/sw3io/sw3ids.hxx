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

#ifndef _SW3IDS_HXX
#define _SW3IDS_HXX
namespace binfilter {

#define	SWGF_NO_FRAMES	0x0001		// keine Layout-Frames einlesen
#define	SWGF_BLOCKNAME	0x0002		// Header hat Textbaustein-Namen
#define	SWGF_HAS_PASSWD	0x0008		// Stream ist passwortgeschuetzt
#define SWGF_HAS_GRFLNK	0x0010		// Stream enthaelt Grafik-Links
#define SWGF_HAS_DDELNK	0x0020		// Stream enthaelt DDE-Links
#define SWGF_HAS_OLELNK	0x0040		// Stream enthaelt OLE-Links
#define	SWGF_PORT_GRAF	0x0080		// Grafiken sollen portabel sein
#define SWGF_HAS_PGNUMS 0x0100		// Stream hat Seitennummern
#define	SWGF_PERSIST	0x0200		// Stream benoetigt SvPersist
#define	SWGF_BAD_LAYOUT	0x4000		// Fehlerhaftes Layout
#define	SWGF_BAD_FILE	0x8000		// Fehler beim Schreiben

#define SWG_FRSTVERSION 0x0001      // erste Version
#define SWG_SPOOLFLAGS  0x0002      // Flag-Byte im SWG_STRINGPOOL-Record
#define SWG_POOLIDS     0x0003      // Pool-IDS im Stringpool
#define	SWG_ZORDER		0x0004		// Z-Order-Record im Drawing Layer
#define	SWG_LAYFRAMES	0x0005		// Layout-Frames
#define SWG_CRYPT	    0x0006      // neues Crypting im PASSWORD-Record
#define SWG_DDESEP      0x0007      // neuer DDE-Token-Seperator
#define SWG_OLEVIS2PAGE 0x0008		// neuer Member des Docs
#define SWG_OLENAME 	0x0009		// neuer Member des OLE-Nodes
#define SWG_SHORTFIELDS 0x000a		// neue Speicherung von Fields
#define SWG_DELETEOLE   0x000b      // unbenutzte OLE-Objecte aus Storage
                                    // loeschen
#define SWG_INTERNET	0x000c		// Fussnoten-Offset
#define SWG_FLYWRAPCHGD 0x000d		// Ausweichmechanik von Txt und Table fuer
                                    // Flys an umbruechen geaendert,
                                    // Invalidierung notwendig.
#define SWG_INETBROWSER	0x000e		// URL-Grf-Nodes, Browse-Flag, Format-Ums.
#define SWG_NONUMLEVEL	0x000f		// NO_NUM umgestellt auf NO_NUMLEVEL
#define SWG_MULTIDB		0x0010		// Datenbankfelder mit DB-Namen
#define SWG_TARGETFRAME	0x0011		// Image-Maps/Target-Frames
#define SWG_USEDDB		0x0012		// In Feldern verwendete Datenbanken
                                    // speichern
#define SWG_INETMACROTAB 0x0013		// InetFelder mit opt. MakroTabelle
#define SWG_NEXTPREVPAGE 0x0014		// PageNumberField (Next/Prev)
#define SWG_URLANDMAP   0x0015		// URL bzw. MAP von den Nodes in das
                                    // Attribut an den Flys verschoben
#define SWG_REGISTER    0x0016		// Spalten-Register/kein Dflt-Target-Frame
#define SWG_NEWNUMRULE  0x0017		// neue Numrules -> Format Erweiterung
#define SWG_DBTABLE		0x0018		// Datenbankfelder mit DB- und Tabellennamen
#define SWG_CONDCOLLS   0x0019		// bedingte Vorlagen am TextNode
#define SWG_WRAPDRAWOBJ 0x0020      // Umfluss um Zeichenobjekte
#define SWG_FMTGETREFFLD 0x0021     // GetRefFelder mit erweitertem Format-Enum
#define SWG_EXPORT31 	0x0022      // Version fuer 3.1 Export aus 4.0

#define SWG_DESKTOP40	0x0101		// StarDesktop 4.0
#define SWG_SVXMACROS	0x0102		// SvxMacros um ScriptType erweitert
#define SWG_TBLCHGMODE	0x0103		// Tabellen Change-Modus
#define SWG_OFFICE40FP2 0x0104		// nur zur Wiedererkennung
#define SWG_EXPORT40	0x0110		// Version fuer 4.0-Export ab 5.0

// Von der 359 bis zur 362 gab es mal die Versionen 0x0120 und 0x0121.
// Dann kam die 363, und es wurde wieder zur 0x0104 zurueckgegangen. Mit
// den Fileformat-Umstellungen kam dann die 0x200.
//#define SWG_OFFICE41	0x0120		// Alles nach StarOffice 4.0 (358)
//#define SWG_SCRIPTURLS  0x0121      // Scripte koennen jetzt URLs sein

#define SWG_NEWFIELDS	0x0200      // Felder mit Numberformatter-Zahlenformat
#define SWG_LONGIDX		0x0201		// Node-Idx sind nung longs uvm.
#define SWG_NEWERFIELDS	0x0202		// Felder aufgeraeumt
#define SWG_HTMLCOLLCHG	0x0203		// Neue-HTML-Poolvorlagen-Ids
#define SWG_FIXEDFLDS	0x0204		// Noch mehr fixe Felder
#define SWG_DATEOFFSET	0x0205		// Offset fuer DateTimeFields
#define SWG_SETEXPFLDCHG 0x0206		// SetExpFieldType - Kapitelweise numer.
#define SWG_HIDDENDRAWOBJS 0x0207	// versteckte Zeichen-Objekte
#define SWG_FIXEDFNFLD 	0x0208		// versteckte Zeichen-Objekte
#define SWG_LONGRECS 	0x0209		// Record-Laenge > 8/16MB
#define SWG_OLEPRTNOTIFY 0x210		// Ab hier OLE Benachrichtigung fuer Printer
                                    // richtig Implementiert.
#define SWG_NUMRELSPACE 0x0211		// relative Numerierung (5.1-Beta2)
#define SWG_NUMRELSPACE2 0x0212		// relative Numerierung
#define SWG_TOXTABS		 0x0213		// relative Numerierung
#define SWG_NEWTOX		 0x0214		// TOXs reworked
#define SWG_NEWTOX2		 0x0215		// TOXs reworked again
#define SWG_FTNANCHORFMT 0x0216		// Ftn-/End-Note Anchor with CharFormat
#define SWG_TOXTABCHAR	0x0217		// TabChar in TOXs
#define SWG_UNICODE1ST	0x0218		// unicode
#define SWG_NEWGRFATTR	0x0219		// new graphic attributes
#define SWG_NEWGRFATTR_FIX	0x0220		// new graphic attributes
#define SWG_VIRTUAL_DEVICE  0x0221      // printer indepemdent formatting
#define SWG_HIDDENOBJLAYER  0x0222      // sdr objs in hidden layer

#define SWG_VERSION     0x0222      // aktuelle Version

// SWG_MAJORVERSION ist die erste Version, zu der die aktuelle Version
// noch kompatibel sein soll.

#define SWG_MAJORVERSION_30 (SWG_FRSTVERSION)
#define SWG_MAJORVERSION_40 (SWG_DESKTOP40)
#define SWG_MAJORVERSION_50 (SWG_LONGIDX)
#define SWG_MAJORVERSION 	(SWG_MAJORVERSION_50)

// Die folgende Versionsnummer wird immer dann hochgezaehlt, wenn das
// Fileformat inkompatibel zu frueheren Version wird, also aeltere SW3
// eine mit der aktuellen Version geschriebene Datei nicht mehr lesen
// koennen.
// Wurde in der 373/374 voruebergehend benutzt

#define SWG_CV_FRSTVER  0x00        // erste "compatbility" Version
//#define SWG_CV_SCNDVER	0x01        // 5.0 FFmt mit 4.0 ClassID

#define SWG_CVERSION    0x00        // aktuelle "compatibility" Version

                                        // Indexwerte fuer String-IDs:
#define	IDX_NO_VALUE	(USHORT) 0xFFFF	// Indexwert nicht belegt
#define	IDX_DFLT_VALUE	(USHORT) 0xFFFE	// Defaultwert verwenden
#define	IDX_COLCNTFMT	(USHORT) 0xFFFD	// Frames: Default-Spaltenformat am Doc
#define	IDX_NOCONV_FF	(USHORT) 0xFFFC	// 0xff nicht mitkonvertieren
                                        // (DB-Felder)
#define	IDX_SPEC_VALUE	(USHORT) 0xFFF0	// ab hier fuer Sonderwerte reserviert
#define	IDX_PAGEFMT   	(USHORT) 0x8000	// Frames: Maske fuer Seitenformat-Index
#define	IDX_LEFTFMT   	(USHORT) 0x4000	// Frames: Maske fuer Leftformat-Index

#define	SWG_OUTLINE		'0' 		// Outline-Numerierung
#define	SWG_FOOTINFO	'1' 		// Fussnoten-Info
#define	SWG_PAGEFOOTINFO '2' 		// Seiten-Fussnoten-Info
#define SWG_NODENUM		'3'			// Absatz-Numerierung
#define SWG_ENDNOTEINFO	'4'			// Endnoten-Info
#define SWG_LINENUMBERINFO '5'		// Zeilennummer-Info
#define SWG_DOCDUMMIES 	'6'			// Dummy-Member des Dokuments
#define SWG_CONFIG		'7'			// Config
#define SWG_PGPREVIEWPRTDATA '8'	// PagePreViewPrintData
#define	SWG_STRINGPOOL	'!' 		// String-Pool
#define	SWG_RECSIZES	'%' 		// Record-Groessen
#define	SWG_OUTLINEEXT	'+' 		// Erweiterung Outline-Numerierung

#define	SWG_BOOKMARKS	'a'			// Alle Bookmarks
//war	SWG_BLOCKNAME	'b'			// Name eines Textbausteins
#define	SWG_CHARFMT		'c' 		// Zeichenformat-Vorlage (wird nicht
                                    // geschrieben)
#define	SWG_DOCSTAT		'd'			// Dokument - Statistik
#define	SWG_BLOCKTEXT	'e'			// Textbaustein-Text
#define	SWG_FRAMEFMT	'f' 		// Frame-Format
#define	SWG_GRFFMT		'g' 		// Grafikformat (wird nicht
                                    // gelesen/geschrieben)
// 						'h'
#define SWG_REPTEXTNODE 'i'			// Wiederholung eines Text-Nodes
#define	SWG_DICTIONARY	'j' 		// Woerterbuecher des Onlinespellings
#define SWG_CONTOUR		'k'			// Contour-Poly-Polygon
#define	SWG_FLYFMT		'l' 		// FlyFrame-Format
#define	SWG_MACRO		'm' 		// Makrodefinition
#define SWG_NUMFMT      'n'         // Numerierungs-Format
#define SWG_SDRFMT		'o' 		// SdrObject-Frameformat
#define	SWG_PAGEDESC	'p' 		// Seitenvorlage
#define SWG_NUMBERFORMATTER 	'q'		// der Numberformatter
#define	SWG_FREEFMT		'r' 		// Frameformat ohne Doc-Anbindung
#define	SWG_SECTFMT		's' 		// Section-Format
#define	SWG_TABLEBOX	't' 		// Tabellen-Zelle
#define SWG_TOXDESCS	'u'         // index descriptions (since 5.2)
#define SWG_NODEREDLINE 'v'			// Redline-Section
#define	SWG_WRONGLIST	'w' 		// Liste falscher Worte
#define SWG_TOXDESC     'x'         // Verzeichnis-Descriptor
#define SWG_TOXDESCS51  'y'         // Verzeichnis-Descriptoren (upto 5.1)
#define SWG_OLELINK     'z'         // OLE-Link Daten

#define	SWG_ATTRIBUTE	'A' 		// Attribut
#define	SWG_BOOKMARK	'B' 		// Bookmark
#define SWG_COMMENT		'C'			// Kommentare
#define SWG_DBNAME		'D'			// DatenbankName
#define	SWG_TABLE		'E' 		// Tabelle
#define	SWG_FLYFRAMES	'F'			// FlyFrames
#define	SWG_GRFNODE		'G' 		// Grafik-Node
// 						'H'
#define	SWG_SECTION		'I' 		// Section
#define	SWG_JOBSETUP	'J' 		// Job-Setup
#define	SWG_MARK		'K'			// Markierung
#define	SWG_TABLELINE	'L' 		// Tabellen-Zeile
#define	SWG_MACROTBL	'M' 		// Makro-Tabelle
#define	SWG_CONTENTS	'N'			// Textteil
#define	SWG_OLENODE		'O' 		// OLE-Node
#define	SWG_PAGEDESCS	'P' 		// Seitenvorlagen
// 						'Q'
#define SWG_NUMRULE     'R'         // Numerierungs-Regelwerk
#define	SWG_ATTRSET		'S'			// Attribut-Set
#define SWG_TEXTNODE	'T' 		// Textnode
#define SWG_LAYOUTINFO	'U'			// Frame-Infos
#define SWG_REDLINES 	'V'			// Redlines
#define	SWG_PASSWORD	'W'			// Passwort
#define SWG_IMAGEMAP	'X'			// Image-Map (fruehr SWG_GRAPHIC_EXT)
#define	SWG_FIELDTYPE	'Y' 		// Feldtyp
#define SWG_EOF			'Z' 		// EOF-Markierung

#ifdef TEST_HUGE_DOCS
#define SWG_TESTHUGEDOCS '?'
#endif

//////////////////////////////////////////////////////////////////////////

// IDs innerhalb eines OLENODE-Records

#define SW_OLE_CHARTNAME	'1'		// Name der Tabelle des Chart-Objectes

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
