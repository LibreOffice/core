/*************************************************************************
 *
 *  $RCSfile: poolfmt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
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
#ifndef _POOLFMT_HXX
#define _POOLFMT_HXX

#include <limits.h>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// POOLCOLL-IDs:
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// !User!    Bereich    ! 0 !               Offset                  !
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//
//  Bereich:                        1 - Text
//                                  2 - Listen
//                                  3 - Sonderbereiche
//                                  4 - Verzeichnisse
//                                  5 - Kapitel / Dokument
//                                  6 - HTML-Vorlagen

// Andere IDs:
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// !User!    Bereich    ! 1 !           Offset                      !
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Bereich:                         0 - Zeichenvorlagen
//                                  1 - Frame-Vorlagen
//                                  2 - Seitenvorlagen
//                                  3 - Absatzformate (?)
//                                  4 - Grafikformate (?)

// FÅr alle IDs gilt:
// Herkunft:                        0 -Pool
//                                  1 -Benutzer
// Offset:                          innerhalb der Gruppe


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// ACHTUNG: neue ID's koennen nur noch am Ende der jeweiligen Gruppe
//          zugefuegt werden. Diese Id's werden vom Reader/Writer ge-
//          lesen und geschrieben. Diese kennen nur den Offset zum Start
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Maske fuer Erkennung von COLLPOOL-Ids:

const USHORT POOLGRP_NOCOLLID       =  (1 << 10);

// POLLCOLL-Gruppen:

const USHORT USER_FMT               =  (1 << 15);
const USHORT POOL_FMT               =  (0 << 15);

const USHORT COLL_TEXT_BITS         =  (1 << 11);
const USHORT COLL_LISTS_BITS        =  (2 << 11);
const USHORT COLL_EXTRA_BITS        =  (3 << 11);
const USHORT COLL_REGISTER_BITS     =  (4 << 11);
const USHORT COLL_DOC_BITS          =  (5 << 11);
const USHORT COLL_HTML_BITS         =  (6 << 11);
const USHORT COLL_GET_RANGE_BITS    = (15 << 11);

// Sonstige Gruppen:

const USHORT POOLGRP_CHARFMT        = (0 << 11) + POOLGRP_NOCOLLID;
const USHORT POOLGRP_FRAMEFMT       = (1 << 11) + POOLGRP_NOCOLLID;
const USHORT POOLGRP_PAGEDESC       = (2 << 11) + POOLGRP_NOCOLLID;
const USHORT POOLGRP_NUMRULE        = (3 << 11) + POOLGRP_NOCOLLID;
//const USHORT POOLGRP_GRFFMT           = (4 << 11) + POOLGRP_NOCOLLID; // ?

// fuer Erkennung ob Benutzer-Vorlage oder nicht:
const USHORT POOL_IDUSER_FMT =
        USHRT_MAX & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID);

inline BOOL IsPoolUserFmt( USHORT nId )
{
    return POOL_IDUSER_FMT ==
                (nId & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID))
        ? TRUE : FALSE;
}


// ID-s fuer die Bereiche
enum RES_POOLFMT
{
RES_POOLFMT_BEGIN = 1,  // HIER GEHT'S LOS !!!
RES_POOL_CHRFMT = RES_POOLFMT_BEGIN,
RES_POOL_FRMFMT,
RES_POOL_TXTCOLL,
RES_POOL_PAGEFMT,

RES_POOL_PARFMT,    // ???
RES_POOL_GRFFMT,    // ???
RES_POOLFMT_END
};

// Bereiche fuer die Id's der einzelnen Formate

// die Id's fuer die Zeichen-Vorlagen
enum RES_POOL_CHRFMT_TYPE
{
RES_POOLCHR_BEGIN = POOLGRP_CHARFMT,
RES_POOLCHR_NORMAL_BEGIN = POOLGRP_CHARFMT,

RES_POOLCHR_FOOTNOTE = RES_POOLCHR_NORMAL_BEGIN,    // Fussnote
RES_POOLCHR_PAGENO,                                 // Seiten/Feld
RES_POOLCHR_LABEL,                                  // Beschriftung
RES_POOLCHR_DROPCAPS,                               // Initialienzeichen
RES_POOLCHR_NUM_LEVEL,                              // Nummerierungszeichen
RES_POOLCHR_BUL_LEVEL,                              // Aufzaehlungszeichen

RES_POOLCHR_INET_NORMAL,                            // Internet normal
RES_POOLCHR_INET_VISIT,                             // Internet besucht
RES_POOLCHR_JUMPEDIT,                               // Platzhalter
RES_POOLCHR_TOXJUMP,                                // Sprung aus Verzeichnis
RES_POOLCHR_ENDNOTE,                                // Endnote
RES_POOLCHR_LINENUM,                                // Zeilennummerierung
RES_POOLCHR_IDX_MAIN_ENTRY,                         // main entry in indexes
RES_POOLCHR_FOOTNOTE_ANCHOR,                        // Fussnotenanker
RES_POOLCHR_ENDNOTE_ANCHOR,                         // Endnotenanker

RES_POOLCHR_NORMAL_END,

RES_POOLCHR_HTML_BEGIN = RES_POOLCHR_BEGIN + 50,    // HTML-Vorlagen
RES_POOLCHR_HTML_EMPHASIS= RES_POOLCHR_HTML_BEGIN,
RES_POOLCHR_HTML_CITIATION,
RES_POOLCHR_HTML_STRONG,
RES_POOLCHR_HTML_CODE,
RES_POOLCHR_HTML_SAMPLE,
RES_POOLCHR_HTML_KEYBOARD,
RES_POOLCHR_HTML_VARIABLE,
RES_POOLCHR_HTML_DEFINSTANCE,
RES_POOLCHR_HTML_TELETYPE,
RES_POOLCHR_HTML_END,

RES_POOLCHR_END = RES_POOLCHR_HTML_END
};


// die Id's fuer die Rahmen-Vorlagen
enum RES_POOL_FRMFMT_TYPE
{
RES_POOLFRM_BEGIN = POOLGRP_FRAMEFMT,

RES_POOLFRM_FRAME = RES_POOLFRM_BEGIN,              // Rahmen
RES_POOLFRM_GRAPHIC,                                // Graphic
RES_POOLFRM_OLE,                                    // OLE
RES_POOLFRM_FORMEL,                                 // Formeln
RES_POOLFRM_MARGINAL,                               // Marginalen
RES_POOLFRM_WATERSIGN,                              // Wasserzeichen
RES_POOLFRM_LABEL,                                  // Etikette

RES_POOLFRM_END
};

// die Id's fuer die Seiten-Vorlagen
enum RES_POOL_PAGEFMT_TYPE
{
RES_POOLPAGE_BEGIN = POOLGRP_PAGEDESC,

RES_POOLPAGE_STANDARD = RES_POOLPAGE_BEGIN,         // Standard-Seite
RES_POOLPAGE_FIRST,                                 // Erste Seite
RES_POOLPAGE_LEFT,                                  // Linke Seite
RES_POOLPAGE_RIGHT,                                 // Rechte Seite
RES_POOLPAGE_JAKET,                                 // Umschlag
RES_POOLPAGE_REGISTER,                              // Verzeichnis
RES_POOLPAGE_HTML,                                  // HTML
RES_POOLPAGE_FOOTNOTE,                              // Fussnote bei Dokumentende
RES_POOLPAGE_ENDNOTE,                               // Endnotensseite

RES_POOLPAGE_END
};

// die Id's fuer die NumRule-Vorlagen
enum RES_POOL_NUMRULE_TYPE
{
RES_POOLNUMRULE_BEGIN = POOLGRP_NUMRULE,
RES_POOLNUMRULE_NUM1 = RES_POOLNUMRULE_BEGIN,       // NumRule Numerierung 1
RES_POOLNUMRULE_NUM2,                               // NumRule Numerierung 2
RES_POOLNUMRULE_NUM3,                               // NumRule Numerierung 3
RES_POOLNUMRULE_NUM4,                               // NumRule Numerierung 4
RES_POOLNUMRULE_NUM5,                               // NumRule Numerierung 5
RES_POOLNUMRULE_BUL1,                               // NumRule Bullets 1
RES_POOLNUMRULE_BUL2,                               // NumRule Bullets 2
RES_POOLNUMRULE_BUL3,                               // NumRule Bullets 3
RES_POOLNUMRULE_BUL4,                               // NumRule Bullets 4
RES_POOLNUMRULE_BUL5,                               // NumRule Bullets 5
RES_POOLNUMRULE_END
};

// die Id's fuer die Absatz-Vorlagen
enum RES_POOL_COLLFMT_TYPE
{
// Gruppe Text
RES_POOLCOLL_TEXT_BEGIN = COLL_TEXT_BITS,

RES_POOLCOLL_STANDARD = RES_POOLCOLL_TEXT_BEGIN,        // Standard
RES_POOLCOLL_TEXT,                                      // Textkoerper
RES_POOLCOLL_TEXT_IDENT,                                // Textkoerper Einzug
RES_POOLCOLL_TEXT_NEGIDENT,                             // Textkoerper neg. Einzug
RES_POOLCOLL_TEXT_MOVE,                                 // Textkoerper Einrueckung
RES_POOLCOLL_GREETING,                                  // Grussformel
RES_POOLCOLL_SIGNATURE,                                 // Unterschrift
RES_POOLCOLL_CONFRONTATION,                             // Gegenueberstellung
RES_POOLCOLL_MARGINAL,                                  // Marginalie

    // Untergruppierung Ueberschriften
RES_POOLCOLL_HEADLINE_BASE,                             // Basis-Ueberschrift
RES_POOLCOLL_HEADLINE1,                                 // Ueberschrift 1
RES_POOLCOLL_HEADLINE2,                                 // Ueberschrift 2
RES_POOLCOLL_HEADLINE3,                                 // Ueberschrift 3
RES_POOLCOLL_HEADLINE4,                                 // Ueberschrift 4
RES_POOLCOLL_HEADLINE5,                                 // Ueberschrift 5
RES_POOLCOLL_HEADLINE6,                                 // Ueberschrift 6
RES_POOLCOLL_HEADLINE7,                                 // Ueberschrift 7
RES_POOLCOLL_HEADLINE8,                                 // Ueberschrift 8
RES_POOLCOLL_HEADLINE9,                                 // Ueberschrift 9
RES_POOLCOLL_HEADLINE10,                                // Ueberschrift 10

RES_POOLCOLL_TEXT_END,


// Gruppe Listen
RES_POOLCOLL_LISTS_BEGIN = COLL_LISTS_BITS,

RES_POOLCOLL_NUMBUL_BASE = RES_POOLCOLL_LISTS_BEGIN,    // Basis-Liste

    // Untergruppe Nummerierung
RES_POOLCOLL_NUM_LEVEL1S,                               // Start Level1
RES_POOLCOLL_NUM_LEVEL1,                                // 1. Level
RES_POOLCOLL_NUM_LEVEL1E,                               // Ende Level1
RES_POOLCOLL_NUM_NONUM1,                                // keine Nummerierung
RES_POOLCOLL_NUM_LEVEL2S,                               // Start 2. Level
RES_POOLCOLL_NUM_LEVEL2,                                // 2. Level
RES_POOLCOLL_NUM_LEVEL2E,                               // Ende 2. Level
RES_POOLCOLL_NUM_NONUM2,                                // keine Nummerierung
RES_POOLCOLL_NUM_LEVEL3S,                               // Start 3. Level
RES_POOLCOLL_NUM_LEVEL3,                                // 3. Level
RES_POOLCOLL_NUM_LEVEL3E,                               // Ende 3. Level
RES_POOLCOLL_NUM_NONUM3,                                // keine Nummerierung
RES_POOLCOLL_NUM_LEVEL4S,                               // Start 4. Level
RES_POOLCOLL_NUM_LEVEL4,                                // 4. Level
RES_POOLCOLL_NUM_LEVEL4E,                               // Ende 4. Level
RES_POOLCOLL_NUM_NONUM4,                                // keine Nummerierung
RES_POOLCOLL_NUM_LEVEL5S,                               // Start 5. Level
RES_POOLCOLL_NUM_LEVEL5,                                // 5. Level
RES_POOLCOLL_NUM_LEVEL5E,                               // Ende 5. Level
RES_POOLCOLL_NUM_NONUM5,                                // keine Nummerierung

    // Untergruppe Aufzaehlung
RES_POOLCOLL_BUL_LEVEL1S,                               // Start Level1
RES_POOLCOLL_BUL_LEVEL1,                                // 1. Level
RES_POOLCOLL_BUL_LEVEL1E,                               // Ende Level1
RES_POOLCOLL_BUL_NONUM1,                                // keine Nummerierung
RES_POOLCOLL_BUL_LEVEL2S,                               // Start 2. Level
RES_POOLCOLL_BUL_LEVEL2,                                // 2. Level
RES_POOLCOLL_BUL_LEVEL2E,                               // Ende 2. Level
RES_POOLCOLL_BUL_NONUM2,                                // keine Nummerierung
RES_POOLCOLL_BUL_LEVEL3S,                               // Start 3. Level
RES_POOLCOLL_BUL_LEVEL3,                                // 3. Level
RES_POOLCOLL_BUL_LEVEL3E,                               // Ende 3. Level
RES_POOLCOLL_BUL_NONUM3,                                // keine Nummerierung
RES_POOLCOLL_BUL_LEVEL4S,                               // Start 4. Level
RES_POOLCOLL_BUL_LEVEL4,                                // 4. Level
RES_POOLCOLL_BUL_LEVEL4E,                               // Ende 4. Level
RES_POOLCOLL_BUL_NONUM4,                                // keine Nummerierung
RES_POOLCOLL_BUL_LEVEL5S,                               // Start 5. Level
RES_POOLCOLL_BUL_LEVEL5,                                // 5. Level
RES_POOLCOLL_BUL_LEVEL5E,                               // Ende 5. Level
RES_POOLCOLL_BUL_NONUM5,                                // keine Nummerierung

RES_POOLCOLL_LISTS_END,


// Sonderbereiche
RES_POOLCOLL_EXTRA_BEGIN = COLL_EXTRA_BITS,

    // Untergruppe Header
RES_POOLCOLL_HEADER = RES_POOLCOLL_EXTRA_BEGIN,         // Header Left&Right
RES_POOLCOLL_HEADERL,                                   // Header Left
RES_POOLCOLL_HEADERR,                                   // Header Right

    // Untergruppe Footer
RES_POOLCOLL_FOOTER,                                    // Footer Left&Right
RES_POOLCOLL_FOOTERL,                                   // Footer Left
RES_POOLCOLL_FOOTERR,                                   // Footer Right

    // Untergruppe Tabelle
RES_POOLCOLL_TABLE,                                     // Tabelle "Inhalt"
RES_POOLCOLL_TABLE_HDLN,                                // Tabellen-Headline


    // Untergruppe Beschriftung
RES_POOLCOLL_LABEL,                                     // Beschriftung-Basis
RES_POOLCOLL_LABEL_ABB,                                 // Beschriftung-Abbildung
RES_POOLCOLL_LABEL_TABLE,                               // Beschriftung-Tabelle
RES_POOLCOLL_LABEL_FRAME,                               // Beschriftung-Rahmen

    // sonstiges
RES_POOLCOLL_FRAME,                                     // Rahmen
RES_POOLCOLL_FOOTNOTE,                                  // Fussnoten
RES_POOLCOLL_JAKETADRESS,                               // UmschlagAdresse
RES_POOLCOLL_SENDADRESS,                                // AbsenderAdresse
RES_POOLCOLL_ENDNOTE,                                   // Endnoten

RES_POOLCOLL_LABEL_DRAWING,                             // Beschriftung-Zeichen-Objekte
RES_POOLCOLL_EXTRA_END,


// Gruppe Verzeichnisse
RES_POOLCOLL_REGISTER_BEGIN = COLL_REGISTER_BITS,

RES_POOLCOLL_REGISTER_BASE = RES_POOLCOLL_REGISTER_BEGIN,   // Basis-Verzeichnis

    // Untergruppe Index-Verzeichnisse
RES_POOLCOLL_TOX_IDXH,                                  // Header
RES_POOLCOLL_TOX_IDX1,                                  // 1. Ebene
RES_POOLCOLL_TOX_IDX2,                                  // 2. Ebene
RES_POOLCOLL_TOX_IDX3,                                  // 3. Ebene
RES_POOLCOLL_TOX_IDXBREAK,                              // Trenner

    // Untergruppe Inhalts-Verzeichnisse
RES_POOLCOLL_TOX_CNTNTH,                                // Header
RES_POOLCOLL_TOX_CNTNT1,                                // 1. Ebene
RES_POOLCOLL_TOX_CNTNT2,                                // 2. Ebene
RES_POOLCOLL_TOX_CNTNT3,                                // 3. Ebene
RES_POOLCOLL_TOX_CNTNT4,                                // 4. Ebene
RES_POOLCOLL_TOX_CNTNT5,                                // 5. Ebene

    // Untergruppe Benutzer-Verzeichnisse:
RES_POOLCOLL_TOX_USERH,                                 // Header
RES_POOLCOLL_TOX_USER1,                                 // 1. Ebene
RES_POOLCOLL_TOX_USER2,                                 // 2. Ebene
RES_POOLCOLL_TOX_USER3,                                 // 3. Ebene
RES_POOLCOLL_TOX_USER4,                                 // 4. Ebene
RES_POOLCOLL_TOX_USER5,                                 // 5. Ebene


RES_POOLCOLL_TOX_CNTNT6,                                // Inhalt 6. Ebene
RES_POOLCOLL_TOX_CNTNT7,                                // Inhalt 7. Ebene
RES_POOLCOLL_TOX_CNTNT8,                                // Inhalt 8. Ebene
RES_POOLCOLL_TOX_CNTNT9,                                // Inhalt 9. Ebene
RES_POOLCOLL_TOX_CNTNT10,                               // Inhalt 10. Ebene

// illustrations index
RES_POOLCOLL_TOX_ILLUSH,                                    // illustrations header
RES_POOLCOLL_TOX_ILLUS1,                                 // illustrations all levels

//  object index
RES_POOLCOLL_TOX_OBJECTH,                               // objects header
RES_POOLCOLL_TOX_OBJECT1,                                // objects all levels

//  tables index
RES_POOLCOLL_TOX_TABLESH,                               // tables header
RES_POOLCOLL_TOX_TABLES1,                                // tables all levels

//  index of authorities
RES_POOLCOLL_TOX_AUTHORITIESH,                          // authorities header
RES_POOLCOLL_TOX_AUTHORITIES1,                           // authorities all levels

// user index 6..10
RES_POOLCOLL_TOX_USER6,                                 // level 6
RES_POOLCOLL_TOX_USER7,                                 // level 7
RES_POOLCOLL_TOX_USER8,                                 // level 8
RES_POOLCOLL_TOX_USER9,                                 // level 9
RES_POOLCOLL_TOX_USER10,                                // level 10

RES_POOLCOLL_REGISTER_END,


// Gruppe Kapitel/Dokument
RES_POOLCOLL_DOC_BEGIN = COLL_DOC_BITS,

RES_POOLCOLL_DOC_TITEL = RES_POOLCOLL_DOC_BEGIN,        // Doc. Titel
RES_POOLCOLL_DOC_SUBTITEL,                              // Doc. UnterTitel

RES_POOLCOLL_DOC_END,

// Gruppe HTML-Vorlagen
RES_POOLCOLL_HTML_BEGIN = COLL_HTML_BITS,

RES_POOLCOLL_HTML_BLOCKQUOTE = RES_POOLCOLL_HTML_BEGIN,
RES_POOLCOLL_HTML_PRE,
RES_POOLCOLL_HTML_HR,
RES_POOLCOLL_HTML_DD,
RES_POOLCOLL_HTML_DT,

RES_POOLCOLL_HTML_END

// Ende der Textformat-Vorlagen Sammlung
};



// erfrage den definierten Parent zu einer POOL-Id
//  returnt:    0           -> Standard
//              USHRT_MAX   -> kein Parent
//              sonst       -> den Parent
USHORT GetPoolParent( USHORT nId );

// erfrage zu einer PoolId den Namen (steht im poolfmt.cxx)
class String;
String& GetDocPoolNm( USHORT nId, String& rFillNm );

inline BOOL IsConditionalByPoolId(USHORT nId)
    {
        return RES_POOLCOLL_TEXT == nId;
    }
// die Id's fuer die Grafik-Formate
/*enum RES_POOL_GRFFMT_TYPE
{
RES_POOLGRF_BEGIN = RES_POOLPAR_END,
RES_POOLGRF_1 = RES_POOLGRF_BEGIN,
RES_POOLGRF_END
};
*/

#endif
