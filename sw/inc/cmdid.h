/*************************************************************************
 *
 *  $RCSfile: cmdid.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/******************************************************************************
Achtung: Ab sofort sind in diesem File keine C++-Kommentare (//) mehr
         erlaubt, da es von swicli.c included wird!
******************************************************************************/

#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif

/* Flags die mittels des Disable-Features in den Slot-Definitionen ausgenutzt
 * werden */
#define SW_DISABLE_ON_PROTECTED_CURSOR  0x00000001
#define SW_DISABLE_ON_MAILBOX_EDITOR    0x00000002


#define FN_FILE                 SID_SW_START
#define FN_EDIT                 (SID_SW_START +  100)
#define FN_VIEW                 (SID_SW_START +  200)
#define FN_INSERT               (SID_SW_START +  300)
#define FN_FORMAT               (SID_SW_START +  400)
#define FN_EXTRA                (SID_SW_START +  600)
#define FN_WINDOW               (SID_SW_START +  700)
#define FN_HELP                 (SID_SW_START +  800)
#define FN_SELECTION            (SID_SW_START +  900)
#define FN_QUERY                (SID_SW_START + 1000)
#define FN_ENVELP               (SID_SW_START + 1050)
#define FN_PARAM                (SID_SW_START + 1100)
#define FN_STAT                 (SID_SW_START + 1180)
#define FN_PRNOPT               (SID_SW_START + 1200)
#define FN_PGPREVIEW            (SID_SW_START + 1250)
#define FN_FRAME                (SID_SW_START + 1300)
#define FN_INSERT2              (SID_SW_START + 1400)
#define FN_FORMAT2              (SID_SW_START + 1600)
#define FN_EDIT2                (SID_SW_START + 1800)
#define FN_QUERY2               (SID_SW_START + 2000)
#define FN_EXTRA2               (SID_SW_START + 2200)
#define FN_PARAM2               (SID_SW_START + 2400)

#define HELP_OFFSET                 1100
#define CMD_STR_OFFSET              2200
#define CMD_STR_OFFSET_MULTILANG    3300
#define CMDID_END                   5500

/*--------------------------------------------------------------------
    Bereich: Datei
 --------------------------------------------------------------------*/
#define FN_CLOSE_FILE           (FN_FILE + 1 )    /* Schliessen */
#define FN_LAUNCH_REGISTRY      (FN_FILE + 2 )    /* Ablage */


#define FN_NEW_GLOBAL_DOC       (FN_FILE + 4 )    /* Globaldokument erzeugen */
#define FN_NEW_FILE             (FN_FILE + 5 )    /* Neu */
#define FN_NEW_FILE_DLG         (FN_FILE + 6 )    /* Neu Dialog */
#define FN_OPEN_FILE            (FN_FILE + 7 )    /* Oeffnen */
#define FN_EDIT_FILE            (FN_FILE + 8 )    /* Oeffnen oder ToTop */

#define FN_INETFILE             (FN_FILE + 9 )    /* Internet File-Dialog */

#define FN_PRINT_FILE           (FN_FILE + 10)    /* Drucken */
#define FN_PRINT_FILE_OPTIONS   (FN_FILE + 11)    /* Drucken Optionen */
#define FN_SAVE_FILE_AS         (FN_FILE + 12)    /* Speichern unter */
#define FN_SAVE_FILE            (FN_FILE + 13)    /* Speichern */
#define FN_SETUP_PRINTER        (FN_FILE + 14)    /* Druckereinstellung */
#define FN_SETUP_PRINTER_DLG    (FN_FILE + 15)    /* Druckereinstellung */
#define FN_SHOW_PREVIEW         (FN_FILE + 16)    /* Druckbild */
#define FN_EXIT                 (FN_FILE + 17)    /* Writer beenden */
#define FN_PRINT_CFG_DLG        (FN_FILE + 18)    /* Optionen Drucken */



#define FN_SAVE_ALL             (FN_FILE + 23)    /* Alle Files speichern */
#define FN_NEW_FILE_DEFAULT     (FN_FILE + 24)    /* Neue Datei mit Standardvorlage */
#define FN_PRINT_FILE_DEFAULT   (FN_FILE + 25)    /* Drucken mit Defaults */
#define FN_LAUNCH_EQ_EDITOR     (FN_FILE + 26)    /* Formel Editor */
#define FN_CHANGE_PRINTER       (FN_FILE + 27)    /* Drucker einstellen */
#define FN_FAX_END              (FN_FILE + 29)    /* Faxen fertig, Id fuer PostMessage */

#define FN_SELECT_DATABASE      (FN_FILE + 30)    /* Selektion Datenbank */
#define FN_DOC_INFO_DLG         (FN_FILE + 31)    /* Dokumentinfo */
#define FN_DOC_MGR_DLG          (FN_FILE + 32)    /* Dokument-Manager */

#define FN_GET_DOCSTAT          (FN_FILE + 33)    /* Dokumentstatistik einzeln auslesen */


#define FN_SAVE_SELECTION       (FN_FILE + 35)    /* Selektion speichern */

#define FN_OUTLINE_TO_IMPRESS   (FN_FILE + 36)  /* Outline zu StarImpress senden */
#define FN_OUTLINE_TO_CLIPBOARD (FN_FILE + 37)  /* Outline in das Clipboad copieren */

#define FN_NEW_HTML_DOC         (FN_FILE + 40 ) /* HTML-Dokument "erzeugen" */

#define FN_APP_START            (FN_FILE + 98)  /* fuer Makro bei App.Start */
#define FN_APP_END              (FN_FILE + 99)  /* fuer Makro bei App.Ende */

/*--------------------------------------------------------------------
    Bereich: Bearbeiten
 --------------------------------------------------------------------*/
#define FN_CLEAR                (FN_EDIT + 1 )    /* Loeschen */
#define FN_COPY                 (FN_EDIT + 2 )    /* Kopieren */
#define FN_CUT                  (FN_EDIT + 3 )    /* Ausschneiden */
#define FN_EDIT_FIELD           (FN_EDIT + 4 )    /* Textbefehl bearbeiten */
#define FN_EDIT_FIELD_DLG       (FN_EDIT + 5 )    /* Textbefehl bearbeiten */
#define FN_EDIT_FILE_INFO       (FN_EDIT + 6 )    /* Dokumentinfo */
#define FN_EDIT_FILE_INFO_DLG   (FN_EDIT + 7 )    /* Dokumentinfo Dialog */
#define FN_EDIT_LINK            (FN_EDIT + 8 )    /* Verknuepfungen */
#define FN_EDIT_LINK_DLG        (FN_EDIT + 9 )    /* Verknuepfungen */
#define FN_GOTO                 (FN_EDIT + 11)    /* Gehe zu */

#define FN_PASTE                (FN_EDIT + 13)    /* Einfuegen */
#define FN_PASTESPECIAL         (FN_EDIT + 14)    /* Verknuepfung einfuegen */
#define FN_PASTESPECIAL_DLG     (FN_EDIT + 15)    /* Verknuepfung einfuegen */

#define FN_NUMBER_BULLETS       (FN_EDIT + 21)    /* Bullets */
#define FN_REPEAT               (FN_EDIT + 22)    /* Letzten Befehl wiederholen */
#define FN_EDIT_IDX_ENTRY_DLG   (FN_EDIT + 23)    /* Index-Entry bearbeiten */
#define FN_UPDATE_FIELDS        (FN_EDIT + 26)    /* Feldinhalte erneuern */
#define FN_EXECUTE_MACROFIELD   (FN_EDIT + 27)    /* Macrofeld ausfuehren */
#define FN_EDIT_FORMULA         (FN_EDIT + 28)    /* Formel in RibbonBar bearbeiten */
#define FN_CALC_TABLE           (FN_EDIT + 29)    /* Tabelle durchrechnen */

/*--------------------------------------------------------------------
    Bereich: Bullets
 --------------------------------------------------------------------*/
#define FN_NUM_BULLET_DOWN      (FN_EDIT + 30)  /* Runterstufen */
#define FN_NUM_BULLET_UP        (FN_EDIT + 31)  /* Raufstufen */
#define FN_NUM_BULLET_PREV      (FN_EDIT + 32)  /* zum vorigen Eintrag */
#define FN_NUM_BULLET_NEXT      (FN_EDIT + 33)  /* zum naechsten Eintrag */
#define FN_NUM_BULLET_MOVEUP    (FN_EDIT + 34)  /* nach oben schieben */
#define FN_NUM_BULLET_MOVEDOWN  (FN_EDIT + 35)  /* nach unten schieben */
#define FN_NUM_BULLET_NONUM     (FN_EDIT + 36)  /* Eintrag ohne Nummer */
#ifndef FN_NUM_BULLET_OFF //in SVX already
#define FN_NUM_BULLET_OFF       (FN_EDIT + 37)  /* Numerierung aus */
#endif


// schon im SVX
//#define FN_NUM_BULLET_ON      (FN_EDIT + 38)  /* Numerierung mit Bullets an */

#define FN_NUM_BULLET_OUTLINE_DOWN      (FN_EDIT + 39)  /* Runterstufen mit Unterpunkten */
#define FN_NUM_BULLET_OUTLINE_UP        (FN_EDIT + 40)  /* Raufstufen mit Unterpunkten */
#define FN_NUM_BULLET_OUTLINE_MOVEUP    (FN_EDIT + 41)  /* nach oben schieben mit Unterpunkten */
#define FN_NUM_BULLET_OUTLINE_MOVEDOWN  (FN_EDIT + 42)  /* nach unten schieben mit Unterpunkten */
#define FN_UPDATE_INPUTFIELDS           (FN_EDIT + 43)  /* Eingabefelder updaten */
// schon im SVX
//#define FN_NUM_NUMBERING_ON   (FN_EDIT + 44)  /* Numerierung an */

#define FN_NUM_OR_NONUM         (FN_EDIT + 46)  /* Nummer ein-/aus */

#define FN_GOTO_NEXT_INPUTFLD   (FN_EDIT + 47)  /* zum naechsten EingabeFeld    */
#define FN_GOTO_PREV_INPUTFLD   (FN_EDIT + 48)  /* zum vorherigen EingabeFeld   */

#define FN_REPEAT_SEARCH        (FN_EDIT + 50)  /* Suche wiederholen */
#define FN_REPEAT_REPLACE       (FN_EDIT + 51)  /* Ersetzen wiederholen */
#define FN_UPDATE_GRAFIC        (FN_EDIT + 53)  /* Grafik aendern */
#define FN_SETTAB_ATCURPOS      (FN_EDIT + 54)  /* Tab an der aktuellen Pos setzen */
#define FN_SET_LR_IND_ATCURPOS  (FN_EDIT + 55)  /* EZE und LR setzen */
#define FN_FRMCNT_TO_BODY       (FN_EDIT + 56)  /* Rahmeninhalt zu Text */
#define FN_UPDATE_TOXBASE       (FN_EDIT + 57)  /* Naechstes Verzeichnis erneuern */

#define FN_REPAGINATE           (FN_EDIT + 61)  /* Neuformatierung erzwingen */
#define FN_EDIT_FOOTNOTE        (FN_EDIT + 62)  /* Fussnote bearbeiten */

#define FN_EDIT_REGION          (FN_EDIT + 65)  /* Bereiche bearbeiten  */
#define FN_GOTO_REFERENCE       (FN_EDIT + 66)  /* Von der Refmark zur Referenz */

// schon im Svx definiert
#define FN_NEXT_BOOKMARK        (FN_EDIT + 68)  /*  */
#define FN_PREV_BOOKMARK        (FN_EDIT + 69)  /*  */

/*Navigator ToolBoxen*/
#define FN_SELECT_BOOKMARK           (FN_EDIT + 70)  /**/
#define FN_ITEM_DOWN                 (FN_EDIT + 71)  /**/
#define FN_ITEM_LEFT                 (FN_EDIT + 72)  /**/
#define FN_ITEM_RIGHT                (FN_EDIT + 73)  /**/
#define FN_ITEM_UP                   (FN_EDIT + 74)  /**/
#define FN_DOWN                      (FN_EDIT + 75)  /**/

#define FN_SELECT_FOOTER             (FN_EDIT + 77)  /**/
#define FN_SELECT_FRAME              (FN_EDIT + 78)  /**/
#define FN_SELECT_HEADER             (FN_EDIT + 79)  /**/
#define FN_PAGENUMBER                (FN_EDIT + 80)  /**/
#define FN_SELECT_AUTO_BOOKMARK      (FN_EDIT + 81)  /**/
#define FN_SELECT_FOOTNOTE           (FN_EDIT + 82)  /**/
#define FN_SELECT_SET_AUTO_BOOKMARK  (FN_EDIT + 83)  /**/
#define FN_SELECT_TABLE              (FN_EDIT + 84)  /**/
#define FN_SELECT_INDEX              (FN_EDIT + 85)  /**/
#define FN_UP                        (FN_EDIT + 86)  /**/


//#define FN_DELETE_REGION           (FN_EDIT + 89) /* Bereiche loeschen    */


#define FN_SELECT_PARA               (FN_EDIT + 97) /* Absatz selektieren */

#define FN_SELECT_CONTENT            (FN_EDIT + 99) /* Navigator - Inhaltstyp */




#define FN_UPDATE_ALL_LINKS          (FN_EDIT2 + 24) /* alle Links updaten */

#define FN_REDLINE_ON                (FN_EDIT2 + 25) /* Redlining anschalten */
#define FN_REDLINE_SHOW              (FN_EDIT2 + 26) /* Redlining anzeigen */
#define FN_REDLINE_COMMENT           (FN_EDIT2 + 27) /* Redlining kommentieren */

#define FN_UPDATE_ALL                (FN_EDIT2 + 28) /* FN_UPDATE_ALL_LINKS,
                                                        FN_UPDATE_FIELDS,
                                                        FN_UPDATE_TOX,
                                                        CalcLayout */

#define FN_REDLINE_ACCEPT            (FN_EDIT2 + 29) /* Redlining annehmen/ablehnen */
#define FN_ATTR_COLUMNS              (FN_EDIT2 + 31) /* SlotId fuer SwFmtCol */
#define FN_EDIT_CURRENT_TOX          (FN_EDIT2 + 32) /* edit current index */
#define FN_EDIT_AUTH_ENTRY_DLG       (FN_EDIT2 + 33) /* edit authorities entry*/
#define FN_UPDATE_CHARTS             (FN_EDIT2 + 34) /* update all charts */

#define FN_EDIT_HYPERLINK            (FN_EDIT2 + 35)    /* edit hyperlink */

/*--------------------------------------------------------------------
    Bereich: Bearbeiten
 --------------------------------------------------------------------*/

#define FN_REFRESH_VIEW         (FN_VIEW + 1)   /* Refresh/Redraw */
#define FN_SHOW_OUTLINE_VIEW    (FN_VIEW + 2)   /* Gliederungsansicht */

#define FN_DRAW_WRAP_DLG        (FN_VIEW + 3)   /* Draw Umlauf-Dlg */

#define FN_RULER                (FN_VIEW + 11)  /* Horizontales Lineal */

#define FN_VIEW_GRAPHIC         (FN_VIEW + 13)  /* Grafiken anzeigen */
#define FN_VIEW_BOUNDS          (FN_VIEW + 14)  /* Begrenzungen */
#define FN_VIEW_FIELDS          (FN_VIEW + 15)  /* Textbefehle */
#define FN_VLINEAL              (FN_VIEW + 16)  /* Vertikales Lineal */
#define FN_VSCROLLBAR           (FN_VIEW + 17)  /* Vertikaler Scrollbar */
#define FN_HSCROLLBAR           (FN_VIEW + 18)  /* Horizontaler Scrollbar */
#define FN_TOOLBOX_TOGGLE       (FN_VIEW + 19)  /* Toolboxinhalte umschalten */
#define FN_VIEWOPTIONS_ON       (FN_VIEW + 20)  /* Gruppe Viewoptions an */
#define FN_VIEWOPTIONS_OFF      (FN_VIEW + 21)  /* Gruppe Viewoptions aus */
#define FN_CONTROLS_ON          (FN_VIEW + 22)  /* Gruppe Bedienelemte an */
#define FN_CONTROLS_OFF         (FN_VIEW + 23)  /* Gruppe Bedienelemte aus */

#define FN_VIEW_META_CHARS      (FN_VIEW + 24)  /* Sonderzeichen anzeigen */
#define FN_VIEW_MARKS           (FN_VIEW + 25)  /* Markierungen anzeigen */
#define FN_VIEW_FIELDNAME       (FN_VIEW + 26)  /* Feldname anzeigen */
#define FN_VIEW_TABLEGRID       (FN_VIEW + 27)  /* Tabellenbegrenzungen anzeigen */

#define FN_LAUNCH_SIM           (FN_VIEW + 28)  /* StarImage starten */
#define FN_SET_PAGE             (FN_VIEW + 29)  /* Seitenvorlage am Absatz setzen */

#define FN_TOOLBOX_SWITCH       (FN_VIEW + 30)  /* Auf Textshell umschalten */


//noch mehr Navigator
#define FN_CONTENT_LB           (FN_VIEW + 32)  /**/
#define FN_SHOW_CONTENT_BOX     (FN_VIEW + 33)  /**/
#define FN_SHOW_ROOT            (FN_VIEW + 34)  /**/
#define FN_DROP_REGION          (FN_VIEW + 35)  /**/
#define FN_OUTLINE_LEVEL        (FN_VIEW + 36)  /**/

#define FN_PRINT_LAYOUT         (FN_VIEW + 37)  /* invertierter BrowseMode */

#define FN_DROP_REGION_LINK     (FN_VIEW + 38)  /**/
#define FN_DROP_REGION_COPY     (FN_VIEW + 39)  /**/

#define FN_SCROLL_NAVIGATION    (FN_VIEW + 40)  /* Navigationscontroller am Scrollbar*/
#define FN_SCROLL_NEXT_PREV     (FN_VIEW + 41)  /* arbeitet den MoveType ab */

#define FN_VIEW_HIDDEN_PARA     (FN_VIEW + 42)  /* Versteckte Absaetze */
#define FN_VIEW_SMOOTH_SCROLL   (FN_VIEW + 43)

#define FN_GLOBAL_SWITCH        (FN_VIEW + 44)  /* umschalten zw. Global und Inhalt*/
#define FN_GLOBAL_EDIT          (FN_VIEW + 45)  /* bearbeiten */
#define FN_GLOBAL_UPDATE        (FN_VIEW + 46)  /* aktualisieren */
#define FN_GLOBAL_OPEN          (FN_VIEW + 47)  /* oeffnen */
#define FN_GLOBAL_SAVE_CONTENT  (FN_VIEW + 48)  /* Inhalt der Verknuepfung mitspeichern */
#define FN_CREATE_NAVIGATION    (FN_VIEW + 49)  /* Navigations-Controller erzeugen */
#define FN_PREVIEW_PRINT_OPTIONS (FN_VIEW + 50)  /* Optionsdialog Preview drucken */
#define FN_PREVIEW_ZOOM          (FN_VIEW + 51)  /* der Tabellencontroller fuer den Zoom */

#define FN_SET_MODOPT_TBLNUMFMT  (FN_VIEW + 52) /* Zahlenerkennung in Tabellen */

/*--------------------------------------------------------------------
    Bereich: Einfuegen
 --------------------------------------------------------------------*/
// schon im Svx definiert
#define FN_INSERT_BOOKMARK      (FN_INSERT + 2 )  /* Sprungmarke einfuegen */

// schon im SVX unter der gleichen Id
//#define FN_INSERT_BREAK       (FN_INSERT + 3 )  /* Umbruch */

#define FN_INSERT_BREAK_DLG     (FN_INSERT + 4 )  /* Umbruch */
#define FN_INSERT_COLUMN_BREAK  (FN_INSERT + 5 )  /* Spaltenumbruch */
#define FN_INSERT_DATE_TIME     (FN_INSERT + 6 )  /* Datum/Uhrzeit */
#define FN_INSERT_FIELD         (FN_INSERT + 8 )  /* Textbefehl */
#define FN_CHANGE_DBFIELD       (FN_INSERT + 9 )  /* Datenbankfeld aendern */

#define FN_INSERT_CAPTION       (FN_INSERT + 10)    /* Beschriftung */
#define FN_INSERT_FOOTNOTE_DLG  (FN_INSERT + 12)    /* Dialog Fussnote */

#define FN_INSERT_REF_FIELD     (FN_INSERT + 13)  /* Refernzfeld einfuegen */

#define FN_INSERT_HYPERLINK     (FN_INSERT + 14)    /* Zeichendialog/HyperlinkPage*/

#define FN_INSERT_INDEX         (FN_INSERT + 16)    /* Stichworteintrag */
#define FN_INSERT_INDEX_DLG     (FN_INSERT + 17)    /* Stichworteintrag */
#define FN_INSERT_LINEBREAK     (FN_INSERT + 18)    /* Zeilenumbruch */

#define FN_INSERT_OBJECT        (FN_INSERT + 21)    /* Object */
#define FN_INSERT_OBJECT_DLG    (FN_INSERT + 22)    /* Object */
#define FN_INSERT_PAGEBREAK     (FN_INSERT + 23)    /* Seitenumbruch */
#define FN_INSERT_PAGENO        (FN_INSERT + 24)    /* Seitennummer */
#define FN_INSERT_PARAGRAPH     (FN_INSERT + 25)    /* Absatz */
#define FN_INSERT_RECORD        (FN_INSERT + 26)    /* Datensatz einfuegen */
#define FN_MERGE_RECORD         (FN_INSERT + 27)    /* Datensatz mischen */
#define FN_INSERT_SYMBOL        (FN_INSERT + 28)    /* Sonderzeichen */
#define FN_POSTIT               (FN_INSERT + 29)    /* PostIt einfuegen/bearbeiten */
#define FN_INSERT_TABLE         (FN_INSERT + 30)    /* Tabelle */
// schon im Svx definiert
#define FN_INSERT_COLS          (FN_INSERT + 32)    /* Spalten einfuegen */
#define FN_INSERT_FRAME_INTERACT (FN_INSERT + 33)   /* Rahmen einfuegen aktiv */
#define FN_INSERT_FRAME         (FN_INSERT + 34)    /* Rahmen einfuegen */

#define FN_INSERT_IDX_ENTRY_DLG (FN_INSERT + 35)    /* Verzeichniseintrag einfuegen */
#define FN_INSERT_FRAME_INTERACT_NOCOL (FN_INSERT + 36) /*insert interactive non column frame*/

#define FN_INSERT_SOFT_HYPHEN   (FN_INSERT + 43)    /* weicher Trenner */
#define FN_INSERT_HARD_SPACE    (FN_INSERT + 44)    /* hartes Space */
#define FN_INSERT_REGION        (FN_INSERT + 45)    /* Bereich einfuegen */



#define FN_TOOL_GROUP           (FN_INSERT + 47)    /* Objekte gruppieren */
#define FN_TOOL_UNGROUP         (FN_INSERT + 48)    /* Gruppierng aufheben */

#define FN_TOOL_ANKER           (FN_INSERT + 49)    /* Draw-Objekt umankern */
#define FN_TOOL_ANKER_PAGE      (FN_INSERT + 50)    /* Draw-Objekt an Seite verankern */
#define FN_TOOL_ANKER_PARAGRAPH (FN_INSERT + 51)    /* Draw-Objekt am Absatz verankern */
#define FN_TOOL_HIERARCHIE      (FN_INSERT + 52)    /* Hierarchie aendern */
#define FN_TOOL_VMIRROR         (FN_INSERT + 53)    /* Objekt in unteren Layer */
#define FN_TOOL_HMIRROR         (FN_INSERT + 54)    /* Objekt in oberen Layer */

//#define FN_QRY_OPEN_DB            (FN_INSERT + 55)    /* Datenbank oeffnen (Basic) */
//#define FN_QRY_DEFINE_DB      (FN_INSERT + 56)    /* Neue Datenbankdefinition anlegen (Basic) */
//#define FN_QRY_SELECT         (FN_INSERT + 57)    /* SQL-Statement angeben (Basic) */
//#define FN_QRY_TO_FIRST_REC       (FN_INSERT + 58)    /* Ersten Datensatz anwaehlen (Basic) */
//#define FN_QRY_TO_NEXT_REC        (FN_INSERT + 59)    /* Naechsten Datensatz anwaehlen (Basic) */
//#define FN_QRY_TO_REC         (FN_INSERT + 60)    /* Bestimmten Datensatz anwaehlen (Basic) */
//#define FN_QRY_GET_RECORD_ID  (FN_INSERT + 61)    /* Datensatznummer erfragen (Basic) */
//#define FN_QRY_GET_COLUMN_NUM (FN_INSERT + 62)    /* Anzahl der Spalten (Basic) */

#define FN_QRY                  (FN_INSERT + 63)    /* Datensatz einfuegen */
#define FN_QRY_MERGE            (FN_INSERT + 64)    /* Datensatz einfuegen (Serienbrief) */
#define FN_QRY_OPEN_TABLE       (FN_INSERT + 65)    /* Datenbank oeffnen (Basic) */
#define FN_TOOL_ANKER_FRAME     (FN_INSERT + 66)    /* Draw-Objekt am Rahmen verankern */
//#define FN_QRY_SWITCH             (FN_INSERT + 67)    /* Auf eine bestimmte Datenbank umschalten */

#define FN_INSERT_SIM           (FN_INSERT + 68)    /* Einfuegen StarImage */
#define FN_INSERT_SMA           (FN_INSERT + 69)    /* Einfuegen StarMath  */

#define FN_QRY_GET_COLUMN_NAME  (FN_INSERT + 70)    /* Anzahl der Spalten (Basic) */
#define FN_QRY_GET_COLUMN_TYPE  (FN_INSERT + 71)    /* Type des Datenbankfeldes ermitteln (Basic) */
#define FN_QRY_GET_COLUMN_TEXT  (FN_INSERT + 72)    /* Inhalt des aktuellen Datensatzes (Basic) */
#define FN_QRY_DEL_DEFINED_DB   (FN_INSERT + 75)    /* Datenbankdefinition loeschen (Basic) */

#define FN_DRAWTEXT_ATTR_DLG    (FN_INSERT + 76)    /* DrawText positionieren */

#define FN_QRY_APPEND_RECORD    (FN_INSERT + 77)    /* Neuen Datensatz anhaengen */
#define FN_QRY_UPDATE_RECORD    (FN_INSERT + 78)    /* Datensatzspalte aendern */
#define FN_QRY_DELETE_RECORD    (FN_INSERT + 79)    /* Datensatz loeschen */
#define FN_QRY_GET_PHYS_RECORD_ID (FN_INSERT + 80)  /* Physikalische (absolute) Datensatznummer erfragen (Basic) */

#define FN_TOOL_ANKER_CHAR       (FN_INSERT + 84)   /* DrawObject zeichengebunden */
#define FN_INSERT_HARDHYPHEN     (FN_INSERT + 85)   /* Bindestrich ohne Umbruch*/

#define FN_QRY_INSERT           (FN_INSERT + 86)    /* Datensatz-Selektion in Text einfuegen */
#define FN_QRY_MERGE_FIELD      (FN_INSERT + 87)    /* Datensatz-Selektion in Felder einfuegen */
#define FN_QRY_INSERT_FIELD     (FN_INSERT + 88)    /* Datenbankfeld einfuegen */

#define FN_INSERT_CTRL          (FN_INSERT + 89)    /* Werkzeugleistencontroller Einfuegen*/
#define FN_INSERT_OBJ_CTRL      (FN_INSERT + 90)    /* Werkzeugleistencontroller Einfuegen/Objekt*/
#define FN_INSERT_FIELD_CTRL    (FN_INSERT + 91)    /* Werkzeugleistencontroller Einfuegen/Feldbefehle*/

#define FN_INSERT_FLD_DATE      (FN_INSERT + 92)
#define FN_INSERT_FLD_TIME      (FN_INSERT + 93)
#define FN_INSERT_FLD_PGNUMBER  (FN_INSERT + 94)
#define FN_INSERT_FLD_PGCOUNT   (FN_INSERT + 95)
#define FN_INSERT_FLD_TOPIC     (FN_INSERT + 96)
#define FN_INSERT_FLD_TITLE     (FN_INSERT + 97)
#define FN_INSERT_FLD_AUTHOR    (FN_INSERT + 98)
#define FN_INSERT_FOOTNOTE      (FN_INSERT + 99)

/*--------------------------------------------------------------------
    Bereich: Einfuegen (2.Teil)
 --------------------------------------------------------------------*/

#define FN_QRY_GET_ALL_DBNAME   (FN_INSERT2 + 1)    /* Alle konfigurierten Datenabanknamen ermitteln */
#define FN_QRY_GET_DBNAME_COUNT (FN_INSERT2 + 2)    /* Anzahl aller konfigurierten Datenabanknamen */
#define FN_QRY_GET_TABLE_NAME   (FN_INSERT2 + 3)    /* Tabellenname einer Datenbank ermitteln */
#define FN_QRY_GET_TABLE_COUNT  (FN_INSERT2 + 4)    /* Anzahl aller Tabellen einer Datenbank ermitteln */

#define FN_INSERT_HEADER        (FN_INSERT2 + 5)    /* Kopfzeile im Html-mode ein/ausschalten */
#define FN_INSERT_FOOTER        (FN_INSERT2 + 6)    /* Fuázeile im Html-mode ein/ausschalten */
#define FN_SBA_BRW_UPDATE       (FN_INSERT2 + 7)    /* Datensaetze in Felder einfuegen */
#define FN_SBA_BRW_INSERT       (FN_INSERT2 + 8)    /* Datensaetze in Text einfuegen */
#define FN_SBA_BRW_MERGE        (FN_INSERT2 + 9)    /* Serienbriefdruck */
#define FN_JAVAEDIT             (FN_INSERT2 + 10)   /* Scriptfeld bearbeiten */
#define FN_INSERT_HRULER        (FN_INSERT2 + 11)   /* horiz. Grafiklinie einfuegen */
#define FN_TOOL_ANKER_AT_CHAR   (FN_INSERT2 + 12)   /* Object zeichengebunden */

#define FN_INSERT_PAGEHEADER    (FN_INSERT2 + 13)   /* Standard Kopfzeile einfuegen */
#define FN_INSERT_PAGEFOOTER    (FN_INSERT2 + 14)   /* Standard Fusszeile einfuegen */

#define FN_INSERT_ENDNOTE           (FN_INSERT2 + 18)   /* Endnote einfuegen*/
#define FN_INSERT_COLUMN_SECTION    (FN_INSERT2 + 19)   /* Spaltigen Bereich einfügen */

#define FN_INSERT_MULTI_TOX         (FN_INSERT2 + 20)   /* insert any TOX */
#define FN_INSERT_AUTH_ENTRY_DLG    (FN_INSERT2 + 21)   /* insert entry for table of authorities*/

#define FN_SECTION_APPENDNODE       (FN_INSERT2 + 22)   /* insert an Paragraph behind the current section*/

/*--------------------------------------------------------------------
    Bereich: Format
 --------------------------------------------------------------------*/

#define FN_AUTOFORMAT_APPLY     (FN_FORMAT + 1 ) /* Autoformat-Optionen anwenden */
#define FN_AUTOFORMAT_AUTO      (FN_FORMAT + 2 ) /* Autoformat waehrend der Eingabe */
#define FN_GROW_FONT_SIZE       (FN_FORMAT + 3 ) /* Groesse */
#define FN_SHRINK_FONT_SIZE     (FN_FORMAT + 4 ) /* Groesse */
#define FN_UNDERLINE_DOUBLE     (FN_FORMAT + 5 ) /* Doppelt unterstreichen */
#define FN_AUTOFORMAT_REDLINE_APPLY (FN_FORMAT + 6 ) /* Autoformat mit Redlining anwenden */
#define FN_SET_SUPER_SCRIPT     (FN_FORMAT + 11) /* Hochstellung */
#define FN_SET_SUB_SCRIPT       (FN_FORMAT + 12) /* Tiefstellung */

#define FN_SET_CASEMAP          (FN_FORMAT + 14) /* CaseMap */
#define FN_SET_LANGUAGE         (FN_FORMAT + 15) /* Sprache */
#define FN_SET_KERNING          (FN_FORMAT + 16) /* Kerning */

#define FN_INDENT_TO_TABSTOP    (FN_FORMAT + 17) /* Einrueckung auf die naechste Tabpostion */
//schon in svxids.hrc
//#define FN_SET_JUSTIFY_PARA   (FN_FORMAT + 21) /* Ausrichtung Absatz */
#define FN_FLIP_HORZ_GRAFIC     (FN_FORMAT + 25) /* Spiegeln horizontal */
#define FN_FLIP_VERT_GRAFIC     (FN_FORMAT + 26) /* Spiegeln vertikal */
#define FN_SET_LINE_SPACE       (FN_FORMAT + 27) /* Zeilenabstand parametrisiert */
#define FN_SET_ADJUST           (FN_FORMAT + 28) /* Ausrichtung parametrisiert */
#define FN_SET_LRMARGIN         (FN_FORMAT + 29) /* linker/rechter Rand */
#define FN_SET_ULMARGIN         (FN_FORMAT + 30) /* oberer/unterer Rand */
#define FN_UNINDENT_TO_TABSTOP  (FN_FORMAT + 31) /* Einrueckung auf die vorgige Tabpostion */

#define FN_SET_HYPHEN_ZONE      (FN_FORMAT + 32) /* Silbentrennung */
#define FN_SET_KEEP_TOGETHER    (FN_FORMAT + 34) /* nicht trennen */
#define FN_SET_KEEP_WITH_NEXT   (FN_FORMAT + 35) /* mit folgendem zus. */
#define FN_SET_WIDOW            (FN_FORMAT + 36) /* Widows */
#define FN_SET_ORPHAN           (FN_FORMAT + 37) /* Orphans */


#define FN_REGISTER_COLLECTION  (FN_FORMAT + 43)  /* Referenzvorlage an der Seite */
#define FN_REGISTER_MODE        (FN_FORMAT + 44)  /* Registermodus an/aus */
#define FN_NUM_FORMAT_TABLE_DLG     (FN_FORMAT + 45)  /* Zahlenformat in Tabelle */
#define FN_FORMAT_BORDER_DLG        (FN_FORMAT + 48)  /* Umrandung */
#define FN_FORMAT_PAGE_COLUMN_DLG   (FN_FORMAT + 49)  /* Spalten pro Seite */
#define FN_FORMAT_BACKGROUND_DLG    (FN_FORMAT + 50)  /* Hintergrund */
#define FN_FORMAT_PAGE              (FN_FORMAT + 51)  /* Seite */
#define FN_FORMAT_PAGE_DLG          (FN_FORMAT + 52)  /* Seite */
#define FN_FORMAT_COLUMN            (FN_FORMAT + 53)  /* Spalten */
#define FN_FORMAT_DROPCAPS          (FN_FORMAT + 54)  /* Initialien */
#define FN_FORMAT_FRAME             (FN_FORMAT + 55)  /* Rahmen */
#define FN_FORMAT_FRAME_DLG         (FN_FORMAT + 56)  /* Rahmen */
#define FN_FORMAT_GRAFIC            (FN_FORMAT + 57)  /* Grafik */
#define FN_FORMAT_GRAFIC_DLG        (FN_FORMAT + 58)  /* Grafik */
#define FN_FORMAT_TABLE             (FN_FORMAT + 59)  /* Tabelle */
#define FN_FORMAT_TABLE_DLG         (FN_FORMAT + 60)  /* Tabelle */
#define FN_FORMAT_OLE_DLG           (FN_FORMAT + 61)  /* OLE bearbeiten */

#define FN_NEW_STYLE_BY_EXAMPLE     (FN_FORMAT + 62)  /* Vorlage erzeugen by Example */
#define FN_UPDATE_STYLE_BY_EXAMPLE  (FN_FORMAT + 63)  /* Vorlage aktualisieren */
#define FN_STYLE_SHEET_FRAME_DLG    (FN_FORMAT + 64)  /* Rahmenvorlage */
#define FN_STYLE_SHEET_DOC_DLG      (FN_FORMAT + 65)  /* Dokumentvorlage */
#define FN_STYLE_SHEET_GRAFIK_DLG   (FN_FORMAT + 66)  /* Dokumentvorlage */
#define FN_FORMAT_FOOTNOTE          (FN_FORMAT + 67)  /* Fussnoten */
#define FN_FORMAT_FOOTNOTE_DLG      (FN_FORMAT + 68)  /* Fussnoten-Dialog */

// schon im Svx definiert
//#define FN_FORMAT_RESET           (FN_FORMAT + 69)  /* Format zuruecksetzen */

#define FN_FRAME_TO_TOP             (FN_FORMAT + 70)  /* Rahmen nach oben */
#define FN_FRAME_TO_BOTTOM          (FN_FORMAT + 71)  /* Rahmen nach unten */

#define FN_FRAME_NOWRAP             (FN_FORMAT + 72)  /* Rahmen kein Umlauf */
#define FN_FRAME_WRAP               (FN_FORMAT + 73)  /* Rahmen Umlauf */
#define FN_FRAME_WRAPTHRU           (FN_FORMAT + 74)  /* Rahmen Durchlauf */

#define FN_FRAME_ALIGN_HORZ_LEFT    (FN_FORMAT + 75)  /* Rahmen horz. links */
#define FN_FRAME_ALIGN_HORZ_RIGHT   (FN_FORMAT + 76)  /* Rahmen horz. rechts */
#define FN_FRAME_ALIGN_HORZ_CENTER  (FN_FORMAT + 77)  /* Rahmen horz. zentriert */
#define FN_FRAME_ALIGN_VERT_TOP     (FN_FORMAT + 78)  /* Rahmen vert. oben */
#define FN_FRAME_ALIGN_VERT_BOTTOM  (FN_FORMAT + 79)  /* Rahmen vert. unten */
#define FN_FRAME_ALIGN_VERT_CENTER  (FN_FORMAT + 80)  /* Rahmen vert. zentriert */

#define FN_SET_FRM_POSITION         (FN_FORMAT + 82)/* Rahmenposition -- " -- */
#define FN_SET_FRM_OPTIONS          (FN_FORMAT + 86)/* Rahmenoptionen -- " -- */

#define FN_SET_PAGE_STYLE           (FN_FORMAT + 93)  /* Anwenden Seitenv. */


#define FN_TABLE_REP                (FN_FORMAT + 99)  /* TableRepresentation */
#define FN_CONVERT_TEXT_TABLE       (FN_FORMAT + 100)  /* Konvertierung Text <-> Tabelle */
#define FN_TABLE_INSERT_ROW         (FN_FORMAT + 101)  /* Tabelle: Zeile einfuegen */
#define FN_TABLE_INSERT_COL         (FN_FORMAT + 102)  /* Tabelle: Spalte einfuegen */
#define FN_TABLE_DELETE_ROW         (FN_FORMAT + 103)  /* Tabelle: Zeile loeschen */
#define FN_TABLE_DELETE_COL         (FN_FORMAT + 104)  /* Tabelle: Spalte loeschen */
#define FN_TABLE_SPLIT_CELLS        (FN_FORMAT + 105)  /* Tabelle: Zellen teilen */
#define FN_TABLE_MERGE_CELLS        (FN_FORMAT + 106)  /* Tabelle: Zellen verbinden */
#define FN_TABLE_SET_ROW_HEIGHT     (FN_FORMAT + 107)  /* Tabelle: Zeilenhoehe setzen */
#define FN_TABLE_SET_COL_WIDTH      (FN_FORMAT + 108)  /* Tabelle: Spaltenbreite setzen */
#define FN_TABLE_SET_ULSPACE        (FN_FORMAT + 109)  /* Tabelle: oberen / unteren Abstand einstellen */
#define FN_OPTIMIZE_TABLE           (FN_FORMAT + 110)  /* ToolBoxItem fuer Optimierung in Tabellen  */
#define FN_TABLE_SET_SHADOW         (FN_FORMAT + 111)  /* Tabelle: Schatten einstellen */
#define FN_TABLE_GOTO_CELL          (FN_FORMAT + 112)  /* Tabelle: Zelle anspringen */
#define FN_TABLE_SELECT_ROW         (FN_FORMAT + 113)  /* Tabelle: Zeile selektieren */
#define FN_TABLE_SELECT_COL         (FN_FORMAT + 114)  /* Tabelle: Spalte selektieren */
#define FN_TABLE_SELECT_ALL         (FN_FORMAT + 115)  /* Tabelle: Tabelle selektieren */
#define FN_TABLE_SET_READ_ONLY      (FN_FORMAT + 116)  /* Tabelle: Tabelle schuetzen */
#define FN_TABLE_SET_READ_ONLY_CELLS (FN_FORMAT + 117)  /* Tabelle: Tabellezellen schuetzen */
#define FN_TABLE_UNSET_READ_ONLY_CELLS  (FN_FORMAT + 119)  /* Tabelle: Tabellezellen schuetzen aufheben */
#define FN_TABLE_HEADLINE_REPEAT    (FN_FORMAT + 120)   /* used in SwXTextTable*/
#define FN_TABLE_ADJUST_CELLS       (FN_FORMAT + 121)  /* Tabelle: Zellen anpassen */

#define FN_FRAME_UP                 (FN_FORMAT + 122)  /* Rahmen um eine Ebene nach oben */
#define FN_FRAME_DOWN               (FN_FORMAT + 123)  /* Rahmen um eine Ebene nach unten */

#define FN_TABLE_INSERT_CHART       (FN_FORMAT + 124)

#define FN_SET_FRM_LRSPACE          (FN_FORMAT + 125)  /* Rahmen Seitenabstand           */
#define FN_SET_FRM_ULSPACE          (FN_FORMAT + 126)  /* Rahmen Kopf-/Fussabstand      */
#define FN_TABLE_SET_LRSPACE        (FN_FORMAT + 127)  /* Tabelle: linker / rechter Abstand einstellen */
#define FN_TABLE_OPTIMAL_HEIGHT     (FN_FORMAT + 128)  /* Zellenhoehe optimal*/

/*--------------------------------------------------------------------
    Bereich: Seitenvorlage
 --------------------------------------------------------------------*/

#define FN_NEW_PAGE_STYLE           (FN_FORMAT + 129) /* Anlegen Seitenvorlage */
#define FN_PAGE_STYLE_SET_LR_MARGIN (FN_FORMAT + 130) /* linker / rechter Rand */
#define FN_PAGE_STYLE_SET_UL_MARGIN (FN_FORMAT + 131) /* oberer / unterer Rand */
#define FN_PAGE_STYLE_SET_PAGE      (FN_FORMAT + 134) /* linke, rechte...  */
#define FN_PAGE_STYLE_SET_PAPER_SIZE (FN_FORMAT + 135) /* Papiergroesse */
#define FN_PAGE_STYLE_SET_PAPER_BIN (FN_FORMAT + 136) /* Druckerschacht */
#define FN_PAGE_STYLE_SET_NUMBER_FORMAT (FN_FORMAT + 137) /* Numerierungsart */
#define FN_PAGE_STYLE_SET_COLS      (FN_FORMAT + 138) /* Spaltenanzahl */


/* OS: Diese Ids werden nur noch fuer die Hilfe benoetigt und sind fuer den
 Dialog durch die Ids ohne _DLG ersetzt*/
#define FN_TABLE_INSERT_COL_DLG     (FN_FORMAT + 142)  /* Tabelle: Dlg. Spalte einfuegen */
#define FN_TABLE_INSERT_ROW_DLG     (FN_FORMAT + 143)  /* Tabelle: Dlg. Zeile einfuegen */



//hier geht's weiter !
/*--------------------------------------------------------------------
    Bereich: Ribbon
 --------------------------------------------------------------------*/

#define FN_SET_BACKGROUND_COLOR     (FN_FORMAT + 150)  /* Hintergrundfarbe */

#define FN_SET_BORDER_POS           (FN_FORMAT + 152)  /* Position Umrandung */
#define FN_SET_BORDER_LINES         (FN_FORMAT + 153)  /* Linienart Umrandung */
#define FN_SET_BORDER_COLOR         (FN_FORMAT + 154)  /* Farbe Umrandung */

#define FN_FORMULA_CALC             (FN_FORMAT + 156)  /* Formelauswahl */
#define FN_FORMULA_CANCEL           (FN_FORMAT + 157)  /* Formel nicht uebernehmen */
#define FN_FORMULA_APPLY            (FN_FORMAT + 158)  /* Formel uebernehmen */

#define FN_TABLE_UNSET_READ_ONLY    (FN_FORMAT + 159)  /* Zellenschutz fuer Tab. aufheben */

#define FN_FORMULA_POSITION         (FN_FORMAT + 160)  /* Positionsanzeige */
#define FN_FORMULA_EDIT             (FN_FORMAT + 161)  /* Editfeld */


#define FN_FRAME_WRAP_IDEAL         (FN_FORMAT + 163)  /* Rahmen Idealer Umlauf */
#define FN_FRAME_WRAPTHRU_TRANSP    (FN_FORMAT + 164)  /* Rahmen Transparenter Durchlauf */

#define FN_FRAME_ALIGN_VERT_ROW_TOP     (FN_FORMAT + 165)  /* Rahmen vert. Zeile oben */
#define FN_FRAME_ALIGN_VERT_ROW_BOTTOM  (FN_FORMAT + 166)  /* Rahmen vert. Zeile unten */
#define FN_FRAME_ALIGN_VERT_ROW_CENTER  (FN_FORMAT + 167)  /* Rahmen vert. Zeile zentriert */

#define FN_FRAME_ALIGN_VERT_CHAR_TOP    (FN_FORMAT + 168)  /* Rahmen vert. Zeichen oben */
#define FN_FRAME_ALIGN_VERT_CHAR_BOTTOM (FN_FORMAT + 169)  /* Rahmen vert. Zeichen unten */
#define FN_FRAME_ALIGN_VERT_CHAR_CENTER (FN_FORMAT + 170)  /* Rahmen vert. Zeichen zentriert */

#define FN_TABLE_SET_DEF_BORDER         (FN_FORMAT + 171)  /* vordefinierte Umrandungen setzen */

#define FN_FRAME_WRAP_LEFT              (FN_FORMAT + 172)  /* Rahmen Umlauf links */
#define FN_FRAME_WRAP_RIGHT             (FN_FORMAT + 173)  /* Rahmen Umlauf links */

#define FN_TABLE_SET_ROW_AUTOHEIGHT (FN_FORMAT + 174)  /* Tabelle: Minimale automatische Zeilenhoehe setzen */


#define FN_WRAP_ANCHOR_ONLY         (FN_FORMAT + 181)  /* Umlauf nur fuer ersten Absatz */

#define FN_TABLE_BALANCE_CELLS      (FN_FORMAT + 182)  /* Spalten gleichmaessig verteilen */
#define FN_TABLE_BALANCE_ROWS       (FN_FORMAT + 183)  /* Zeilen gleichmaessig verteilen */

#define FN_FRAME_WRAP_CONTOUR       (FN_FORMAT + 184)  /* Rahmen Kunturumlauf */

#define FN_TABLE_VERT_NONE          (FN_FORMAT + 185)  /* vertikale Ausrichtung in Tabellenzellen */
#define FN_TABLE_VERT_CENTER        (FN_FORMAT + 186)  /*   -"-  */
#define FN_TABLE_VERT_BOTTOM        (FN_FORMAT + 187)  /*  -"-  */
#define FN_TABLE_SET_VERT_ALIGN     (FN_FORMAT + 188)  /*  -"-  */
#define FN_TABLE_MODE_FIX           (FN_FORMAT + 189)  /* Tabellenmodus */
#define FN_TABLE_MODE_FIX_PROP      (FN_FORMAT + 190)  /*  -"-  */
#define FN_TABLE_MODE_VARIABLE      (FN_FORMAT + 191)  /*  -"-  */

#define FN_TABLE_AUTOSUM            (FN_FORMAT + 195)  /* Autosumme*/
#define FN_SET_CONTROL_HANDLER      (FN_FORMAT + 199)  /* Macro setzen */

#define FN_IS_IMAGE                  (FN_FORMAT2 + 6)   /* Numerierung : mit Grafik?*/

#define FN_GOTO_NEXT_REGION          (FN_FORMAT2 + 9)   /* Naechsten Bereich anspringen */
#define FN_GOTO_PREV_REGION          (FN_FORMAT2 + 10)  /* Vorherigen " */

#define FN_GET_SBXCONTROL            (FN_FORMAT2 + 11)  /* SbxObject rausreichen */
#define FN_ABSTRACT_NEWDOC           (FN_FORMAT2 + 12)  /* Abstract in neuem Doc */
#define FN_ABSTRACT_STARIMPRESS      (FN_FORMAT2 + 13)  /* Abstract an StarImpress */







#define FN_NUMBER_FORMAT                    (FN_FORMAT2 + 120) /* Boxen/NumberFormatter eistellen */
#define FN_NUMBER_STANDARD                  (FN_FORMAT2 + 121)
#define FN_NUMBER_TWODEC                    (FN_FORMAT2 + 123)
#define FN_NUMBER_SCIENTIFIC                (FN_FORMAT2 + 124)
#define FN_NUMBER_DATE                      (FN_FORMAT2 + 125)
#define FN_NUMBER_TIME                      (FN_FORMAT2 + 126)
#define FN_NUMBER_CURRENCY                  (FN_FORMAT2 + 127)
#define FN_NUMBER_PERCENT                   (FN_FORMAT2 + 128)



#define FN_FRAME_CHAIN                      (FN_FORMAT2 + 136)
#define FN_FRAME_UNCHAIN                    (FN_FORMAT2 + 137)

#define FN_NUMBER_NEWSTART                  (FN_FORMAT2 + 138)
#define FN_NUMBER_NEWSTART_AT               (FN_FORMAT2 + 139)

#define FN_FRAME_MIRROR_ON_EVEN_PAGES       (FN_FORMAT2 + 140)
#define FN_GRAPHIC_MIRROR_ON_EVEN_PAGES     (FN_FORMAT2 + 141)

#define FN_TABLE_SPLIT_TABLE                (FN_FORMAT2 + 142)
#define FN_SYNC_LABELS                      (FN_FORMAT2 + 143)


#define FN_TABLE_RELATIVE_WIDTH             (FN_FORMAT2 + 147)  /* Tabelle: relative Breite - UNO */
#define FN_TABLE_WIDTH                      (FN_FORMAT2 + 148)  /* Tabelle: Breite - UNO */
#define FN_TABLE_IS_RELATIVE_WIDTH          (FN_FORMAT2 + 149)  /* Tabelle: ist Breite relativ?- UNO */

#define FN_INC_INDENT_OFFSET                (FN_FORMAT2 + 150)
#define FN_DEC_INDENT_OFFSET                (FN_FORMAT2 + 151)

#define FN_TABLE_MERGE_TABLE                (FN_FORMAT2 + 152)

/*--------------------------------------------------------------------
    Bereich: Extras
 --------------------------------------------------------------------*/

#define FN_LINE_NUMBERING_DLG       (FN_EXTRA + 2 )   /* Zeilennumerierung */
#define FN_THESAURUS_DLG            (FN_EXTRA + 3 )   /* Thesaurus */
#define FN_HYPHENATE_OPT_DLG        (FN_EXTRA + 5 )   /* Silbentrennung */
#define FN_ADD_UNKNOWN              (FN_EXTRA + 6 )   /* Woerter lernen */
#define FN_DICTIONARY_DLG           (FN_EXTRA + 8 )   /* Woerterbuecher */
#define FN_NUMBERING                (FN_EXTRA + 9 )   /* Nummerierung/Bullets */
#define FN_NUMBERING_DLG            (FN_EXTRA + 10)   /* Nummerierung/Bullets */
#define FN_NUMBERING_OUTLINE        (FN_EXTRA + 11)   /* Gliederungsnumerierung */
#define FN_NUMBERING_OUTLINE_DLG    (FN_EXTRA + 12)   /* Gliederungsnumerierung */
#define FN_SORTING_DLG              (FN_EXTRA + 14)   /* Sortieren */
#define FN_CALCULATE                (FN_EXTRA + 15)   /* Berechnen */
#define FN_GENERATE_TOC_DLG         (FN_EXTRA + 16)   /* Inhaltsverzeichnis */
#define FN_GENERATE_INDEX_DLG       (FN_EXTRA + 17)   /* Stichwortverzeichnis */
#define FN_GENERATE_DIR_DLG         (FN_EXTRA + 18)   /* Verzeichnis */
#define FN_GLOSSARY_DLG             (FN_EXTRA + 20)   /* Textbausteine */
#define FN_MACRO_CHOOSER            (FN_EXTRA + 21)   /* Makro aufnehmen */
#define FN_SPELLING_DLG             (FN_EXTRA + 22)   /* Rechtschreibung */

#define FN_PLAY_MACRO               (FN_EXTRA + 23)   /* Makro abspielen */
#define FN_LAUNCH_BASIC             (FN_EXTRA + 25)   /* Makro abspielen */
#define FN_EXPAND_GLOSSARY          (FN_EXTRA + 28)   /* Textbausteine expandieren */
#define FN_CONFIG_TOOLBOX           (FN_EXTRA + 29)   /* Konfiguration Toolbox */
#define FN_CONFIG_MENU              (FN_EXTRA + 30)   /* Konfiguration Menu */
#define FN_CONFIG_KEY               (FN_EXTRA + 31)   /* Konfiguration Tastatur */
#define FN_CHANGE_PAGENUM           (FN_EXTRA + 34)   /* Seitennummer aendern */

#define FN_MACRO_POPUP              (FN_EXTRA + 37)   /* Alle Macrofunktionen */
#define FN_BULLET                   (FN_EXTRA + 38)   /* Bullet-Liste */


//  Bereich: Gloassaries

// schon im Svx definiert

//#define FN_GET_GLOSSARY_GROUP_COUNT (FN_EXTRA + 42)     /* Anzahl der Bausteingruppen */

#define FN_AUTO_CORRECT             (FN_EXTRA + 49 )  /* Autocorrect aus Basic */
#define FN_AUTO_CORRECT_DLG         (FN_EXTRA + 50 )  /* Dialog Autocorrect */

#define FN_UPDATE_TOX               (FN_EXTRA + 53)   /* alle Verzeichnisse aktualisieren */
#define FN_UPDATE_CUR_TOX           (FN_EXTRA + 54)   /* aktuelles Verzeichnisse aktualisieren */
#define FN_REMOVE_CUR_TOX           (FN_EXTRA + 55)  /* remove the current TOX*/

#define FN_NAVIGATION_PI_GOTO_PAGE  (FN_EXTRA + 59 )  /* Seitenanwahl aus Navi-PI */


#define FN_LETTER_WIZZARD           (FN_EXTRA + 60 )
#define FN_FAX_WIZZARD              (FN_EXTRA + 61 )
#define FN_MEMO_WIZZARD             (FN_EXTRA + 62 )
#define FN_AGENDA_WIZZARD           (FN_EXTRA + 63 )

#define FN_SET_BASIC_METRIC         (FN_EXTRA + 80) /* Defaultmetrik Basic setzen */

#define FN_RESERVED_9               (FN_EXTRA + 86)   /* Platzhalter */
#define FN_RESERVED_8               (FN_EXTRA + 87)   /* Platzhalter */
#define FN_RESERVED_7               (FN_EXTRA + 88)   /* Platzhalter */
#define FN_RESERVED_6               (FN_EXTRA + 89)   /* Platzhalter */
#define FN_RESERVED_5               (FN_EXTRA + 90)   /* Platzhalter */
#define FN_RESERVED_4               (FN_EXTRA + 91)   /* Platzhalter */
#define FN_RESERVED_3               (FN_EXTRA + 92)   /* Platzhalter */
#define FN_RESERVED_2               (FN_EXTRA + 93)   /* Platzhalter */
#define FN_RESERVED_1               (FN_EXTRA + 94)   /* Platzhalter */

#define FN_COLLECTION_GROUP_CNT     (FN_EXTRA + 96)
#define FN_COLLECTION_GROUP_IDX     (FN_EXTRA + 97)
#define FN_COLL_TYPE                (FN_EXTRA + 98)   /* Typ fuer GlobalDoc-Collection*/
#define FN_COLL_ADD                 (FN_EXTRA + 99)

#define FN_COLL_TITLE               (FN_EXTRA2 + 1)  /*  Bereichsname oder Index-Title */
#define FN_SHADOWCURSOR             (FN_EXTRA2 + 4)  /* Shadow Cursor ein/ausschalten */


#define FN_VIEW_IN_FRAME            (FN_EXTRA2 + 8)  /* die View befindet sich in einem FrameDoc*/

#define FN_UNO_PARA_STYLE           (FN_EXTRA2 + 9)  // jetzt kommen diverse UNO-Ids fuer die
#define FN_UNO_PAGE_STYLE           (FN_EXTRA2 + 10)  // PropertyMap

#define FN_UNO_FRAME_STYLE          (FN_EXTRA2 + 12)
#define FN_UNO_NUM_START_VALUE      (FN_EXTRA2 + 13)
#define FN_UNO_NUM_LEVEL            (FN_EXTRA2 + 14)
#define FN_UNO_NUM_RULES            (FN_EXTRA2 + 15)
#define FN_UNO_DOCUMENT_INDEX_MARK  (FN_EXTRA2 + 16)
#define FN_UNO_DOCUMENT_INDEX       (FN_EXTRA2 + 17)
#define FN_UNO_TEXT_FIELD           (FN_EXTRA2 + 18)
#define FN_UNO_TEXT_TABLE           (FN_EXTRA2 + 19)
#define FN_UNO_CELL                 (FN_EXTRA2 + 20)
#define FN_UNO_TEXT_FRAME           (FN_EXTRA2 + 21)
#define FN_UNO_REFERENCE_MARK       (FN_EXTRA2 + 22)
#define FN_UNO_TEXT_SECTION         (FN_EXTRA2 + 23)
#define FN_UNO_FOOTNOTE             (FN_EXTRA2 + 24)
#define FN_UNO_ENDNOTE              (FN_EXTRA2 + 25)
#define FN_UNO_RANGE_COL_LABEL      (FN_EXTRA2 + 26)
#define FN_UNO_RANGE_ROW_LABEL      (FN_EXTRA2 + 27)
#define FN_UNO_TABLE_COLUMS         (FN_EXTRA2 + 28)
#define FN_UNO_TABLE_BORDER         (FN_EXTRA2 + 29)
#define FN_UNO_TABLE_COLUMN_SEPARATORS      (FN_EXTRA2 + 30)
#define FN_UNO_TABLE_COLUMN_RELATIVE_SUM    (FN_EXTRA2 + 31)
#define FN_UNO_TABLE_CELL_BACKGROUND        (FN_EXTRA2 + 32)
#define FN_UNO_ROW_HEIGHT                   (FN_EXTRA2 + 33)
#define FN_UNO_ROW_AUTO_HEIGHT              (FN_EXTRA2 + 34)
#define FN_UNO_HEADER                       (FN_EXTRA2 + 35)
#define FN_UNO_HEADER_LEFT                  (FN_EXTRA2 + 36)
#define FN_UNO_HEADER_RIGHT                 (FN_EXTRA2 + 37)
#define FN_UNO_FOOTER                       (FN_EXTRA2 + 38)
#define FN_UNO_FOOTER_LEFT                  (FN_EXTRA2 + 39)
#define FN_UNO_FOOTER_RIGHT                 (FN_EXTRA2 + 40)
#define FN_UNO_HEADER_BACKGROUND            (FN_EXTRA2 + 41)
#define FN_UNO_HEADER_BOX                   (FN_EXTRA2 + 42)
#define FN_UNO_HEADER_LR_SPACE              (FN_EXTRA2 + 43)
#define FN_UNO_HEADER_SHADOW                (FN_EXTRA2 + 44)
#define FN_UNO_FOOTER_BACKGROUND            (FN_EXTRA2 + 45)
#define FN_UNO_FOOTER_BOX                   (FN_EXTRA2 + 46)
#define FN_UNO_FOOTER_LR_SPACE              (FN_EXTRA2 + 47)
#define FN_UNO_FOOTER_SHADOW                (FN_EXTRA2 + 48)
#define FN_UNO_HEADER_BODY_DISTANCE         (FN_EXTRA2 + 49)
#define FN_UNO_HEADER_IS_DYNAMIC_DISTANCE   (FN_EXTRA2 + 50)
#define FN_UNO_FOOTER_BODY_DISTANCE         (FN_EXTRA2 + 51)
#define FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE   (FN_EXTRA2 + 52)
#define FN_UNO_HEADER_SHARE_CONTENT         (FN_EXTRA2 + 53)
#define FN_UNO_FOOTER_SHARE_CONTENT         (FN_EXTRA2 + 54)
#define FN_UNO_HEADER_HEIGHT                (FN_EXTRA2 + 55)
#define FN_UNO_FOOTER_HEIGHT                (FN_EXTRA2 + 56)
#define FN_UNO_HEADER_ON                    (FN_EXTRA2 + 57)
#define FN_UNO_FOOTER_ON                    (FN_EXTRA2 + 58)
#define FN_UNO_FOLLOW_STYLE                 (FN_EXTRA2 + 59)

#define FN_API_CALL                         (FN_EXTRA2 + 60)

#define FN_UNO_IS_PHYSICAL                  (FN_EXTRA2 + 61)
#define FN_UNO_IS_AUTO_UPDATE               (FN_EXTRA2 + 62)
#define FN_UNO_DISPLAY_NAME                 (FN_EXTRA2 + 63)

#define FN_UNO_WRAP                         (FN_EXTRA2 + 64)
#define FN_UNO_ANCHOR_TYPES                 (FN_EXTRA2 + 65)
#define FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL (FN_EXTRA2 + 66)
#define FN_UNO_PARA_CONDITIONAL_STYLE_NAME  (FN_EXTRA2 + 67)

#define FN_UNO_CATEGORY                     (FN_EXTRA2 + 68)
#define FN_UNO_IS_NUMBER                    (FN_EXTRA2 + 69)
#define FN_UNO_TEXT_WRAP                    (FN_EXTRA2 + 70)
#define FN_UNO_ANCHOR_TYPE                  (FN_EXTRA2 + 71)
/*--------------------------------------------------------------------
    Bereich: Fenster
 --------------------------------------------------------------------*/

#define FN_SOURCEVIEW               SID_SOURCEVIEW
/*--------------------------------------------------------------------
    Bereich: Hilfe
 --------------------------------------------------------------------*/


#define FN_LAUNCH_HELP              (FN_HELP + 1)    /* Hilfe */
#define FN_SHOW_HELP_INDEX          (FN_HELP + 2)    /* Hilfeindex */
#define FN_SHOW_KEYBOARD_HELP       (FN_HELP + 3)    /* Hilfe */
#define FN_SHOW_HELP_MANUAL         (FN_HELP + 4)    /* Hilfe */
#define FN_SHOW_UPDATE_HELP         (FN_HELP + 5)    /* Hilfe */

#define FN_BALLOON_HELP             (FN_HELP + 7)    /* Toggle Balloon Help */
#define FN_HELP_POPUP               (FN_HELP + 8)    /* Popup Help */


/*--------------------------------------------------------------------
    Bereich: Traveling & Selektion
 --------------------------------------------------------------------*/

#define FN_CHAR_LEFT                (FN_SELECTION + 1 ) /* */
#define FN_CHAR_RIGHT               (FN_SELECTION + 2 ) /* */
#define FN_LINE_UP                  (FN_SELECTION + 3 ) /* */
#define FN_LINE_DOWN                (FN_SELECTION + 4 ) /* */
#define FN_START_OF_LINE            (FN_SELECTION + 5 ) /* StartOfLine */
#define FN_END_OF_LINE              (FN_SELECTION + 6 ) /* EndOfLine */
#define FN_START_OF_DOCUMENT        (FN_SELECTION + 7 ) /* StartOfDocument */
//schon in svxids.hrc
//#define FN_END_OF_DOCUMENT            (FN_SELECTION + 8 ) /* EndOfDocument */
#define FN_START_OF_NEXT_PAGE       (FN_SELECTION + 9 ) /* StartOfNextPage ??? */
#define FN_END_OF_NEXT_PAGE         (FN_SELECTION + 10) /* ??? */
#define FN_START_OF_PREV_PAGE       (FN_SELECTION + 11) /* StartOfPrevPage ??? */
#define FN_END_OF_PREV_PAGE         (FN_SELECTION + 12) /* ??? */
#define FN_START_OF_PAGE            (FN_SELECTION + 13) /* StartOfPage */
#define FN_END_OF_PAGE              (FN_SELECTION + 14) /* EndOfPage */
#define FN_START_OF_WINDOW          (FN_SELECTION + 15) /* StartOfWindow */
#define FN_END_OF_WINDOW            (FN_SELECTION + 16) /* EndOfWindow */
#define FN_START_OF_COLUMN          (FN_SELECTION + 17) /* StartOfColumn */
#define FN_END_OF_COLUMN            (FN_SELECTION + 18) /* EndOfColumn */
#define FN_START_OF_PARA            (FN_SELECTION + 19) /* StartOfPara */
#define FN_END_OF_PARA              (FN_SELECTION + 20) /* EndOfPara */
#define FN_NEXT_WORD                (FN_SELECTION + 21) /* NextWord */
#define FN_PREV_WORD                (FN_SELECTION + 22) /* PrevWord */
#define FN_NEXT_SENT                (FN_SELECTION + 23) /* NextSentence */
#define FN_PREV_SENT                (FN_SELECTION + 24) /* PrevSentence */
#define FN_DELETE                   (FN_SELECTION + 25) /* Delete */
#define FN_BACKSPACE                (FN_SELECTION + 26) /* Backspace */
#define FN_DELETE_SENT              (FN_SELECTION + 27) /* DeleteToEndOfSentence */
#define FN_DELETE_BACK_SENT         (FN_SELECTION + 28) /* DeleteToStartOfSentence */
#define FN_DELETE_WORD              (FN_SELECTION + 29) /* DeleteToEndOfWord */
#define FN_DELETE_BACK_WORD         (FN_SELECTION + 30) /* DeleteToStartOfWord */
#define FN_DELETE_LINE              (FN_SELECTION + 31) /* DeleteToEndOfLine */
#define FN_DELETE_BACK_LINE         (FN_SELECTION + 32) /* DeleteToStartOfLine */
#define FN_DELETE_PARA              (FN_SELECTION + 33) /* DeleteToEndOfPara */
#define FN_DELETE_BACK_PARA         (FN_SELECTION + 34) /* DeleteToStartOfPara */
#define FN_DELETE_WHOLE_LINE        (FN_SELECTION + 35) /* DeleteLine ??? */
#define FN_SET_INS_MODE             (FN_SELECTION + 36) /* Einfuegemodus / Overwrite */
#define FN_PAGEUP                   (FN_SELECTION + 37) /* PageUpWithCursor */
#define FN_PAGEDOWN                 (FN_SELECTION + 38) /* PageDownWithCursor */
#define FN_SET_ADD_MODE             (FN_SELECTION + 39) /* Selektionsmodus */
#define FN_SET_EXT_MODE             (FN_SELECTION + 40) /* Selektionsmodus */
#define FN_ESCAPE                   (FN_SELECTION + 41) /* Normal */
#define FN_SHIFT_BACKSPACE          (FN_SELECTION + 42) /* wie Backspace */
#define FN_SELECT_WORD              (FN_SELECTION + 43) /* Wort selektieren */

#define FN_GOTO_NEXT_OBJ            (FN_SELECTION + 44)   /* N„chstes Objekt anspringen */
#define FN_GOTO_PREV_OBJ            (FN_SELECTION + 45)   /* Vorhergehendes Objekt anspringen */

#define FN_START_TABLE              (FN_SELECTION + 47) /* an den Anfang der Tabelle */
#define FN_END_TABLE                (FN_SELECTION + 48) /* an das Ende der Tabelle */
#define FN_NEXT_TABLE               (FN_SELECTION + 49) /* zur vorhergehenden Tabelle */
#define FN_PREV_TABLE               (FN_SELECTION + 50) /* zur naechsten Tabelle */

#define FN_START_OF_NEXT_COLUMN     (FN_SELECTION + 51) /* zum Anfang der naechsten Spalte */
#define FN_END_OF_NEXT_COLUMN       (FN_SELECTION + 52) /* zum Ende der naechsten Spalte */
#define FN_START_OF_PREV_COLUMN     (FN_SELECTION + 53) /* zum Anfang der vorhergehenden Spalte */
#define FN_END_OF_PREV_COLUMN       (FN_SELECTION + 54) /* zum Ende der vorhergehenden Spalte */

#define FN_FOOTNOTE_TO_ANCHOR       (FN_SELECTION + 55) /* aus der Fussnote zum Anchor */
#define FN_NEXT_FOOTNOTE            (FN_SELECTION + 56) /* zur naechsten Fussnote */
#define FN_PREV_FOOTNOTE            (FN_SELECTION + 57) /* zur vorhergehenden Fussnote */

#define FN_CNTNT_TO_NEXT_FRAME      (FN_SELECTION + 58) /* aus dem Inhalt zum naechsten Rahmen */
#define FN_FRAME_TO_ANCHOR          (FN_SELECTION + 59) /* vom Rahmen zum Anker */

#define FN_TO_HEADER                (FN_SELECTION + 60) /* Toggle zwischen Content und Kopfzeile */
#define FN_TO_FOOTER                (FN_SELECTION + 61) /* Toggle zwischen Content und Fusszeile */

#define FN_IDX_MARK_TO_IDX          (FN_SELECTION + 62) /* von der Verzeichnismarkierung zum Verzeichnis */





#define FN_PREV_PARA                (FN_SELECTION + 74)
#define FN_NEXT_PARA                (FN_SELECTION + 75)

#define FN_GOTO_NEXT_MARK           (FN_SELECTION + 76)   /* N„chsten Platzhalter anspringen */
#define FN_GOTO_PREV_MARK           (FN_SELECTION + 77)   /* Vorhergehenden Platzhalter anspringen */

// schon im SVX unter der gleichen Id
//#define FN_START_DOC_DIRECT           (FN_SELECTION + 78)   /* Direkt zum Dokanfang springen */
//#define FN_END_DOC_DIRECT         (FN_SELECTION + 79)   /* Direkt zum Dokanfang springen */

#define FN_NEXT_TOXMARK             (FN_SELECTION + 83)   /* zur naechsten Verz. Markierung */
#define FN_PREV_TOXMARK             (FN_SELECTION + 84)   /* zur vorherigen Verz. Markierung */
#define FN_NEXT_TBLFML              (FN_SELECTION + 85)   /* zur naechsten Tabellenformel */
#define FN_PREV_TBLFML              (FN_SELECTION + 86)   /* zur vorherigen Tabellenformel */
#define FN_NEXT_TBLFML_ERR          (FN_SELECTION + 87)   /* zur naechsten fehl. Tabellenformel */
#define FN_PREV_TBLFML_ERR          (FN_SELECTION + 88)   /* zur vorherigen fehl. Tabellenformel */


/*--------------------------------------------------------------------
    QUERY-Block
 --------------------------------------------------------------------*/



#define FN_IS_MODIFIED              (FN_QUERY +16)      /* Dokument geaendert? */
#define FN_RESET_MODIFIED           (FN_QUERY +17)      /* Geaendert-Flag zuruecksetzen */

#define FN_GET_DOC_INFO             (FN_QUERY +22)      /* Wert Dokumentinfo erfragen */
#define FN_GET_ERRNO                (FN_QUERY +26)      /* Error-Nummer abfragen */
#define FN_GET_DOCMAN_PATH          (FN_QUERY +27)      /* Pfad zu einer Gruppe im Docmgr erfragen */

#define FN_TXTATR_INET              (FN_QUERY +29)      /* INet-Attribut */
#define FN_EXECUTE_DRAG             (FN_QUERY +30)      /* D&D starten */

#define FN_GET_PRINT_AREA           (FN_QUERY +32)      /* Attribut fuer druckbaren Seitenbereich besorgen */



// #define FN_IS_START_OF_PAGE         (FN_QUERY +40)       /**/
// #define FN_IS_END_OF_PAGE           (FN_QUERY +41)       /**/

#define FN_GET_PARAGRAPH            (FN_QUERY +48)      /*  Paragraph-Object von der Textshell
                                                            an die View weiterreichen*/
// hier ist Schluss!!!


/*--------------------------------------------------------------------
    Bereich: Envelope
 --------------------------------------------------------------------*/

#define FN_ENVELOP                  (FN_ENVELP    )
#define FN_LABEL                    (FN_ENVELP + 1)
#define FN_BUSINESS_CARD            (FN_ENVELP + 2)

/*--------------------------------------------------------------------
    Bereich: Parameter
 --------------------------------------------------------------------*/

#define FN_PARAM_TABLE_COLS         (FN_PARAM)
#define FN_PARAM_TABLE_ROWS         (FN_PARAM+1)

#define FN_PARAM_SEARCH_WORDONLY    (FN_PARAM+5)
#define FN_PARAM_SEARCH_EXACT       (FN_PARAM+6)
#define FN_PARAM_SEARCH_REGEXP      (FN_PARAM+7)
#define FN_PARAM_SEARCH_BACKWARDS   (FN_PARAM+8)
#define FN_PARAM_SEARCH_FINDALL     (FN_PARAM+9)
#define FN_PARAM_SEARCH_SELECTION   (FN_PARAM+10)
#define FN_PARAM_SEARCH_TEXT        (FN_PARAM+11)
#define FN_PARAM_SEARCH_TEXTREPLACE (FN_PARAM+12)

#define FN_PARAM_PARADLGLIMITS      (FN_PARAM+15)
#define FN_PARAM_FRMMODE            (FN_PARAM+16)

#define FN_PARAM_ADDPRINTER         (FN_PARAM+18)
#define FN_PARAM_GRID               (FN_PARAM+19)
#define FN_PARAM_DOCDISP            (FN_PARAM+20)
#define FN_PARAM_ELEM               (FN_PARAM+21)
#define FN_PARAM_SWTEST             (FN_PARAM+22)

#define FN_PARAM_FTN_INFO           (FN_PARAM+23)

#define FN_PARAM_REGION_NAME            (FN_PARAM+24)
#define FN_PARAM_REGION_CONDITION       (FN_PARAM+25)
#define FN_PARAM_REGION_HIDDEN          (FN_PARAM+26)
#define FN_PARAM_REGION_PROTECT         (FN_PARAM+27)

#define FN_PARAM_INSERT_AFTER           (FN_PARAM+29)
#define FN_PARAM_FILTER                 (FN_PARAM+30)
#define FN_PARAM_INDEX_NAME             (FN_PARAM+31)
#define FN_PARAM_INDEX_OPTIONS          (FN_PARAM+32)
#define FN_PARAM_INDEX_FORMAT           (FN_PARAM+33)
#define FN_PARAM_INDEX_LEVEL            (FN_PARAM+34)
#define FN_PARAM_INDEX_TEMPLATE         (FN_PARAM+35)
#define FN_PARAM_INDEX_ALTSTR           (FN_PARAM+36)
#define FN_PARAM_INDEX_PRIMKEY          (FN_PARAM+37)
#define FN_PARAM_INDEX_SECKEY           (FN_PARAM+38)
#define FN_PARAM_WIDTH                  (FN_PARAM+39)
#define FN_PARAM_LOCATION               (FN_PARAM+40)
#define FN_PARAM_DIR                    (FN_PARAM+41)
#define FN_PARAM_COUNT                  (FN_PARAM+42)
#define FN_PARAM_COLOR                  (FN_PARAM+43)

#define FN_PARAM_TABLE_NAME             (FN_PARAM+44)
#define FN_PARAM_TABLE_WIDTH            (FN_PARAM+45)
#define FN_PARAM_TABLE_ULSPACE          (FN_PARAM+46)
#define FN_PARAM_TABLE_LRSPACE          (FN_PARAM+47)
#define FN_PARAM_TABLE_SHADOW           (FN_PARAM+48)
#define FN_PARAM_TABLE_ALIGN            (FN_PARAM+49)
#define FN_PARAM_TABLE_HEADLINE         (FN_PARAM+50)
#define FN_PARAM_TABLE_SPACE            (FN_PARAM+51)
#define FN_PARAM_TABLE_COLUMNS          (FN_PARAM+52)

#define FN_PARAM_GRF_CONNECT            (FN_PARAM+54)


#define FN_PARAM_EVENT                  (FN_PARAM+68)
#define FN_PARAM_URL                    (FN_PARAM+69)
#define FN_PARAM_GRF_REALSIZE           (FN_PARAM+70)
#define FN_PARAM_GRF_DIALOG             (FN_PARAM+71)
#define FN_PARAM_GRF_CLIENTMAP          (FN_PARAM+74) /* ClientMap */
#define FN_PARAM_GRF_SERVERMAP          (FN_PARAM+75) /* ServerMap */
#define FN_PARAM_GRF_TARGETFRAME        (FN_PARAM+76) /* TargetFrame */
#define FN_INET_FIELD_MACRO             (FN_PARAM+77) /* Id fuer URL-Feld-Macros*/

#define FN_PARAM_PRINTER                (FN_PARAM+78) /* Drucker* */
#define FN_PARAM_STDFONTS               (FN_PARAM+79) /* ConfigItem Standardfonts */

#define FN_PARAM_WRTSHELL               (FN_PARAM2) /* SwWrtShell */

#define FN_COND_COLL                    (FN_PARAM2+1) /* Item fuer bed. Vorlagen */
#define FN_PARAM_SELECTION              (FN_PARAM2+2) /* selektiertes Wort fuer Format/Zeichen/Hyperlink */

#define FN_PARAM_ACT_NUMBER             (FN_PARAM2+3) /* PointerItem die aktuelle NumRule */
#define FN_PARAM_CHILD_LEVELS           (FN_PARAM2+4) /* Werden Child-Levels benutzt ?*/

#define FN_PARAM_NUM_PRESET             (FN_PARAM2+5) /* vorgewaehlte Numerierung*/

#define FN_PARAM_HEIGHT                 (FN_PARAM2+6) /* Param fuer Hoehe */
#define FN_PARAM_DIST                   (FN_PARAM2+7) /* und Breite, wg. metrischer Umwandlung */

#define FN_PARAM_SHADOWCURSOR           (FN_PARAM2+8) /* Fuer ShadowCursor Optionen */

#define FN_PARAM_ACT_NUMLEVEL           (FN_PARAM2+9) /* BYTE-Item mit aktuellen NumLevel */

#define FN_PARAM_9                      (FN_PARAM2+10)
#define FN_PARAM_10                     (FN_PARAM2+11)

#define FN_TEXT_RANGE                   (FN_PARAM2+12) /* TextRange Property*/

#define FN_PARAM_CRSR_IN_PROTECTED      (FN_PARAM2+13) /* Cursor in geschuetzten Bereichen*/
#define FN_PARAM_TOX_TYPE               (FN_PARAM2+14) /* TOX type in tox dialog*/
#define FN_PARAM_LINK_DISPLAY_NAME      (FN_PARAM2+15) /* LinkDisplayName property*/
#define FN_PARAM_NUM_LEVEL              (FN_PARAM2+16) /* rtf filter*/
/*--------------------------------------------------------------------
    Bereich: Druckoptionen
 --------------------------------------------------------------------*/

#define FN_PRNOPT_GRAPHIC           (FN_PRNOPT + 1)
#define FN_PRNOPT_TABLE             (FN_PRNOPT + 2)
#define FN_PRNOPT_LEFT_PAGE         (FN_PRNOPT + 3)
#define FN_PRNOPT_RIGHT_PAGE        (FN_PRNOPT + 4)
#define FN_PRNOPT_REVERSE           (FN_PRNOPT + 5)
#define FN_PRNOPT_PAPER_FROM_SETUP  (FN_PRNOPT + 6)
#define FN_PRNOPT_POSTITS           (FN_PRNOPT + 8)

#define FN_PRNOPT_BEGIN  FN_PRNOPT_GRAPHIC
#define FN_PRNOPT_END    FN_PRNOPT_POSTITS

/*--------------------------------------------------------------------
    Beschreibung: Status : nicht mehr als 19!
 --------------------------------------------------------------------*/

#define FN_STAT_PAGE                (FN_STAT + 1)
#define FN_STAT_TEMPLATE            (FN_STAT + 2)
#define FN_STAT_ZOOM                (FN_STAT + 3)
#define FN_STAT_INSOVER             (FN_STAT + 4)
#define FN_STAT_SELMODE             (FN_STAT + 5)
#define FN_STAT_HYPERLINKS          (FN_STAT + 6)  /* Hyperlinks ausfuehren */
#define FN_STAT_CONTEXT             (FN_STAT + 7)
#define FN_STAT_BOOKMARK            (FN_STAT + 8)  /* Fuer Popup Bookmarks*/

/*--------------------------------------------------------------------
    Bereich: Seitenvorschau
 --------------------------------------------------------------------*/

#define FN_SHOW_TWO_PAGES           (FN_PGPREVIEW + 1)
#define FN_SHOW_FOUR_PAGES          (FN_PGPREVIEW + 2)
#define FN_PRINT_PAGEPREVIEW        (FN_PGPREVIEW + 3)

/*--------------------------------------------------------------------
    Bereich: Rahmenattribute
 --------------------------------------------------------------------*/

#define FN_OPAQUE                       (FN_FRAME + 1)
#define FN_SET_PROTECT                  (FN_FRAME + 2)
#define FN_SURROUND                     (FN_FRAME + 3)
#define FN_VERT_ORIENT                  (FN_FRAME + 4)
#define FN_HORI_ORIENT                  (FN_FRAME + 5)
#define FN_SET_FRM_NAME                 (FN_FRAME + 6)
#define FN_KEEP_ASPECT_RATIO            (FN_FRAME + 7)
#define FN_GRF_KEEP_ZOOM                (FN_FRAME + 8)

#define FN_IID_COL1                     (FN_FRAME + 13)
#define FN_IID_COL2                     (FN_FRAME + 14)
#define FN_IID_COL3                     (FN_FRAME + 15)
#define FN_IID_COLLEFT                  (FN_FRAME + 16)
#define FN_IID_COLRIGHT                 (FN_FRAME + 17)

#define FN_SET_FRM_ALT_NAME             (FN_FRAME + 18)


//Member-Ids fuer Fill/SetVariable an Items
#define         MID_STYLE       0xe0
#define         MID_PWIDTH      0xe1
#define         MID_ADJUST      0xe2
#define         MID_TDIST       0xe3
#define         MID_BDIST       0xe4
#define         MID_LINES       0xe5
#define         MID_CHARS       0xe6
#define         MID_DIST        0xe7

#define         MID_1           0xe8
#define         MID_2           0xe9
#define         MID_3           0xea
#define         MID_4           0xeb
#define         MID_5           0xec
#define         MID_6           0xed
#define         MID_7           0xee
#define         MID_8           0xef
#define         MID_9           0xf0
#define         MID_A           0xf1
#define         MID_B           0xf2
#define         MID_C           0xf3
#define         MID_D           0xf4
#define         MID_E           0xf5
#define         MID_F           0xf6
#define         MID_10          0xf7
#define         MID_11          0xf8
#define         MID_12          0xf9
#define         MID_13          0xfa

#ifndef SID_SW_DOCMAN_PATH
#define SID_SW_DOCMAN_PATH                              (SID_OFASLOTS_START +   6)
#endif

// defines fuer Event-Zuweisung per Macro
#define MAC_EVENT_MOUSEOVER         0x01            //
#define MAC_EVENT_MOUSECLICK        0x02            // Hyperlink ausloesen
#define MAC_EVENT_MOUSEOUT          0x03            //
#define MAC_EVENT_OBJECT_SELECT     0x04            // Obj. selektieren
#define MAC_EVENT_KEYINPUT_ALPHA    0x05            //
#define MAC_EVENT_KEYINPUT_NOALPHA  0x06            //
#define MAC_EVENT_FRM_RESIZE        0x07            //
#define MAC_EVENT_FRM_MOVE          0x08            //
#define MAC_EVENT_IMAGE_LOAD        0x09            //
#define MAC_EVENT_IMAGE_ABORT       0x0a            //
#define MAC_EVENT_IMAGE_ERROR       0x0b            //

