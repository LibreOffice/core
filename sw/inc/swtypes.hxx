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
#ifndef _SWTYPES_HXX
#define _SWTYPES_HXX
#include <tools/solar.h>
#include <SwGetPoolIdFromName.hxx>

#ifndef INCLUDED_LIMITS_H
#include <limits.h>     //fuer LONG_MAX
#define INCLUDED_LIMITS_H
#endif

#ifdef PM20
#include <stdlib.h>
#endif
#include <com/sun/star/uno/Reference.h>
#include "swdllapi.h"
#include <i18npool/lang.h>
// wenn das hier geaendert wird, dann auch im globals.hrc aendern!!!
//#define SW_FILEFORMAT_40 SOFFICE_FILEFORMAT_40

namespace com { namespace sun { namespace star {
    namespace linguistic2{
        class XDictionaryList;
        class XSpellChecker1;
        class XHyphenator;
        class XThesaurus;
    }
    namespace beans{
        class XPropertySet;
    }
}}}
namespace utl{
    class TransliterationWrapper;
}

class Size;
class ResMgr;
class UniString;
class ByteString;
class SwPathFinder;
class Graphic;
class OutputDevice;
class CharClass;
class LocaleDataWrapper;
class CollatorWrapper;

typedef long SwTwips;
#define INVALID_TWIPS   LONG_MAX
#define TWIPS_MAX       (LONG_MAX - 1)

#define MM50   283  // 1/2 cm in TWIPS
// --> OD 2004-06-18 #i19922# - constant for minimal page header/footer height of 1mm
const SwTwips cMinHdFtHeight = 56;

#define MINFLY 23   //Minimalgroesse fuer FlyFrms
#define MINLAY 23   //Minimalgroesse anderer Frms

// Default-Spaltenabstand zweier Textspalten entspricht 0.3 cm
#define DEF_GUTTER_WIDTH (MM50 / 5 * 3)

//Minimale Distance (Abstand zum Text) fuer das BorderAttribut, damit
//die aligned'en Linien nicht geplaettet werden.
//28 Twips == 0,4mm
#define MIN_BORDER_DIST 28

    /* minimaler Dokmentrand */
const SwTwips lMinBorder = 1134;

//Die Wiesenbreite links neben und ueber dem Dokument.
//Die halbe Wiesenbreite ist der Abstand zwischen den Seiten.
#define DOCUMENTBORDER  568L
#define GAPBETWEENPAGES 284L

//initiale UndoActionCount
#define UNDO_ACTION_COUNT 20

// Konstante Strings
SW_DLLPUBLIC extern UniString aEmptyStr;            // ""
extern ByteString aEmptyByteStr;    // ""
SW_DLLPUBLIC extern UniString aDotStr;          // '.'

//Zum Einfuegen von Beschriftungen (wie bzw. wo soll eingefuegt werden).
//Hier weil ein eigenes hxx nicht lohnt und es sonst nirgendwo so recht
//hinpasst.
enum SwLabelType
{
    LTYPE_TABLE,    //Beschriftung einer Tabelle
    LTYPE_OBJECT,   //Beschriftung von Grafik oder OLE
    LTYPE_FLY,      //Beschriftung eines (Text-)Rahmens
    LTYPE_DRAW      //Beschriftung eines Zeichen-Objektes
};


const sal_uInt8 OLD_MAXLEVEL = 5;
const sal_uInt8 MAXLEVEL = 10;      //Ehemals numrule.hxx
const sal_uInt8 NO_NUM      = 200;  //Ehemals numrule.hxx
//const sal_uInt8 NO_NUMBERING = 201;  //Ehemals numrule.hxx    //#outline level,removed by zhaojianwei
//const sal_uInt8 NO_INIT       = 202;  //Ehemals numrule.hxx

// fuer Absaetze mit NO_NUM aber auf unterschiedlichen Leveln
// DAMIT entfaellt das NO_NUM !!!!
const sal_uInt8 NO_NUMLEVEL  = 0x20;    // wird mit den Levels verodert


/*
 * Nette Funktionen als MACRO oder inline
 */

/* ein KiloByte sind 1024 Byte */
#define KB 1024

#define SET_CURR_SHELL( shell ) CurrShell aCurr( shell )

// pPathFinder wird von der UI initialisiert. Die Klasse liefert alle
// benoetigten Pfade.
extern SwPathFinder *pPathFinder;

// Werte fuer die Einzuege an der Nummerierung und BulletListe
// (fuer die weiteren Ebenen sind die Werte mit den Ebenen+1 zu
//  multiplizieren; Ebenen 0..4 !!!)

// -> #i30312#
const sal_uInt16 lBullIndent = 1440/4;
const short lBullFirstLineOffset = -lBullIndent;
const sal_uInt16 lNumIndent = 1440/4;
const short lNumFirstLineOffset = -lNumIndent;
const short lOutlineMinTextDistance = 216; // 0.15 inch = 0.38 cm
// <- #i30312#

// Anzahl der SystemField-Types vom SwDoc
#define INIT_FLDTYPES   32
// Anzahl der vordefinierten Seq-Feldtypen. Es handelt sich dabei
// immer um die letzen Felder vor INIT_FLDTYPES
#define INIT_SEQ_FLDTYPES   4

//Die ehemaligen Rendevouz-Ids leben weiter:
//Es gibt Ids fuer die Anker (SwFmtAnchor) und ein paar weitere die nur fuer
//Schnittstellen Bedeutung haben (SwDoc).
enum RndStdIds
{
    FLY_AT_PARA,        // anchored at paragraph
    FLY_AS_CHAR,        // anchored as character
    FLY_AT_PAGE,        // anchored at page
    FLY_AT_FLY,         // anchored at frame
    FLY_AT_CHAR,        // anchored at character
                        //Der Rest wird nur fuer SS benutzt.
    RND_STD_HEADER,
    RND_STD_FOOTER,
    RND_STD_HEADERL,
    RND_STD_HEADERR,
    RND_STD_FOOTERL,
    RND_STD_FOOTERR,

    RND_DRAW_OBJECT     // ein Draw-Object !! nur fuer die SwDoc-Schnittstelle!
};


extern ResMgr* pSwResMgr;           // steht in swapp0.cxx
#define SW_RES(i)       ResId(i,*pSwResMgr)
#define SW_RESSTR(i)    UniString(ResId(i,*pSwResMgr))

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellChecker1 > GetSpellChecker();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenator >    GetHyphenator();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XThesaurus >     GetThesaurus();
::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >         GetLinguPropertySet();

// reutns the twip size of this graphic
SW_DLLPUBLIC Size GetGraphicSizeTwip( const Graphic&, OutputDevice* pOutDev );


// Seperator fuer Sprunge im Dokument auf verschiedene Inhalttype
const sal_Unicode cMarkSeperator = '|';
extern const sal_Char* __FAR_DATA pMarkToTable;             // Strings stehen
extern const sal_Char* __FAR_DATA pMarkToFrame;             // im Init.cxx
extern const sal_Char* __FAR_DATA pMarkToRegion;
SW_DLLPUBLIC extern const sal_Char* __FAR_DATA pMarkToOutline;
extern const sal_Char* __FAR_DATA pMarkToText;
extern const sal_Char* __FAR_DATA pMarkToGraphic;
extern const sal_Char* __FAR_DATA pMarkToOLE;

#ifndef DB_DELIM                            // Ist in OFA definiert!!!
#define DB_DELIM ((sal_Unicode)0xff)        // Datenbank <-> Tabellen-Trenner
#endif


typedef sal_uInt16 SetAttrMode;

namespace nsSetAttrMode
{
    const SetAttrMode SETATTR_DEFAULT        = 0x0000;  // default
    const SetAttrMode SETATTR_DONTEXPAND     = 0x0001;  // TextAttribute nicht weiter expand.
    const SetAttrMode SETATTR_DONTREPLACE    = 0x0002;  // kein anderes TextAttrib verdraengen

    const SetAttrMode SETATTR_NOTXTATRCHR    = 0x0004;  // bei Attr ohne Ende kein 0xFF einfuegen
    const SetAttrMode SETATTR_NOHINTADJUST   = 0x0008;  // keine Zusammenfassung von Bereichen.
    const SetAttrMode SETATTR_NOFORMATATTR   = 0x0010;  // nicht zum FormatAttribut umwandeln
    const SetAttrMode SETATTR_DONTCHGNUMRULE = 0x0020;  // nicht die NumRule veraendern
    const SetAttrMode SETATTR_APICALL        = 0x0040;  // called from API (all UI related
                                                        // functionality will be disabled)
    /// force hint expand (only matters for hints with CH_TXTATR)
    const SetAttrMode SETATTR_FORCEHINTEXPAND= 0x0080;
}

//Umrechnung Twip<-> 1/100 mm fuer UNO

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))
#define TWIP_TO_MM100_UNSIGNED(TWIP)     ((((TWIP)*127L+36L)/72L))
#define MM100_TO_TWIP_UNSIGNED(MM100)    ((((MM100)*72L+63L)/127L))

#define SW_ISPRINTABLE( c ) ( c >= ' ' && 127 != c )

#ifndef SW_CONSTASCII_DECL
#define SW_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SW_CONSTASCII_DEF
#define SW_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif


#define CHAR_HARDBLANK      ((sal_Unicode)0x00A0)
#define CHAR_HARDHYPHEN     ((sal_Unicode)0x2011)
#define CHAR_SOFTHYPHEN     ((sal_Unicode)0x00AD)
#define CHAR_RLM            ((sal_Unicode)0x200F)
#define CHAR_LRM            ((sal_Unicode)0x200E)
#define CHAR_ZWSP           ((sal_Unicode)0x200B)
#define CHAR_ZWNBSP         ((sal_Unicode)0x2060)


// returns the APP - CharClass instance - used for all ToUpper/ToLower/...
SW_DLLPUBLIC CharClass& GetAppCharClass();
SW_DLLPUBLIC LanguageType GetAppLanguage();


#if 0
// I18N doesn't get this right, can't specify more than one to ignore
#define SW_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_KANA | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_WIDTH )
#else
#define SW_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )
#endif

SW_DLLPUBLIC CollatorWrapper& GetAppCollator();
SW_DLLPUBLIC CollatorWrapper& GetAppCaseCollator();

SW_DLLPUBLIC const ::utl::TransliterationWrapper& GetAppCmpStrIgnore();

// --> OD 2004-06-30 #i28701# - moved from <frame.hxx>
//fuer Prepare() zur Benachrichtigung des Inhaltes durch das Layout auf
//dem kurzen Dienstweg.
//Der Inhalt sorgt dafuer, dass beim naechsten Aufruf von ::Format() das
//minimal notwendige berechnet wird.
enum PrepareHint
{
    PREP_BEGIN,             //BEGIN
    PREP_CLEAR = PREP_BEGIN,//Komplett neuformatieren.
    PREP_WIDOWS_ORPHANS,    //Nur Witwen- und Waisen-Regelung pruefen und ggf.
                            //Aufspalten.
    PREP_FIXSIZE_CHG,       //Die FixSize hat sich veraendert.
    PREP_FOLLOW_FOLLOWS,    //Follow ist jetzt moeglicherweise direkter
                            //Nachbar.
    PREP_ADJUST_FRM,        //Groesse per Grow/Shrink Ausrichten ohne zu
                            //Formatieren.
    PREP_FLY_CHGD,          //Ein FlyFrm hat sich (Groesse) veraendert.
    PREP_FLY_ATTR_CHG,      //Ein FlyFrm hat seine Attribute veraendert
                            //(z.B. Umlauf)
    PREP_FLY_ARRIVE,        //Ein FlyFrm ueberlappt den Bereich jetzt neu.
    PREP_FLY_LEAVE,         //Ein FlyFrm hat den Bereich verlassen.
    PREP_FTN,               //Fussnoten-Invalidierung
    PREP_POS_CHGD,          //Position des Frm hat sich verandert
                            //(Zum Fly-Umbruch pruefen). Im void* des Prepare()
                            //wird ein sal_Bool& uebergeben, dieser zeigt mit sal_True,
                            //dass ein Format ausgefuehrt wurde.
    PREP_UL_SPACE,          //UL-Space hat sich veraendert, TxtFrms muessen
                            //den Zeilenabstand neu kalkulieren.
    PREP_MUST_FIT,          //Frm passen machen (aufspalten) auch wenn die
                            //Attribute es nicht erlauben (z.B. zusammenhalten).
    PREP_WIDOWS,            // Ein Follow stellt fest, dass in ihm die Orphans-
                            // regel zuschlaegt und verschickt an seinen
                            // Vorgaenger (Master/Follow) ein PREP_WIDOWS
    PREP_QUOVADIS,          // Wenn eine Fussnote _zwischen_ zwei Absaetzen
                            // aufgespalten werden muss, dann muss der
                            // letzte auf der Seite noch ein QUOVADIS bekommen
                            // damit er den Text hineinformatiert.
    PREP_BOSS_CHGD,         // Wenn ein Frm die Spalte/Seite wechselt, wird dieses
                            // Zusatzprepare zum POS_CHGD im MoveFwd/Bwd
                            // verschickt (Ftn-Nummern joinen etc.)
                            // Die Richtung wird ueber pVoid mitgeteilt:
                            //     MoveFwd: pVoid == 0
                            //     MoveBwd: pVoid == pOldPage
    PREP_SWAP,              //Grafiken Swappen, fuer Grafiken im sichtbaren
                            //Bereich.
    PREP_REGISTER,          //Registerhaltige Frames invalidieren
    PREP_FTN_GONE,          //Ein Follow verliert eine Fussnote, ggf. kann seine erste
                            //Zeile hochrutschen
    PREP_MOVEFTN,           //eine Fussnote wechselt die Seite, der Inhalt bekommt
                            //zunaechst eine Hoehe von Null, damit nicht zuviel
                            //Unruhe entsteht. Beim Formatieren prueft er, ob er
                            //ueberhaupt passt und wechselt ggf. unbemerkt wieder
                            //die Seite.
    PREP_ERGOSUM,           //wg. Bewegung in FtnFrms QuoVadis/ErgoSum pruefen
    PREP_END                //END
};
// <--

#endif
