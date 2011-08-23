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
#ifndef _EDITSH_HXX
#define _EDITSH_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif
#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVXSWAFOPT_HXX
#include <bf_svx/swafopt.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>	// fuer Basisklasse
#endif
#ifndef _ITABENUM_HXX
#include <itabenum.hxx>
#endif
#ifndef _SWDBDATA_HXX
#include <swdbdata.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif
namespace com { namespace sun { namespace star { namespace uno {
    template < class > class Sequence;
}}}}
class PolyPolygon;
class DateTime;
class CommandExtTextInputData;

class Graphic; 			// fuer GetGraphic
class Font;
namespace binfilter {
class SvStringsDtor;
class SvInPlaceObjectRef;
class SvInPlaceObject;
class SvNumberFormatter;
class SfxPoolItem;
class SfxItemSet;
class BfGraphicObject; // fuer GetGraphicObj


class SwDoc;
class SvxLinkManager;
class SvxAutoCorrect;

class SwField;			// fuer Felder
class SwFieldType;
class SwDDEFieldType;
class SwNewDBMgr;

struct SwDocStat;
class SfxDocumentInfo;

class SvStringsSort;
class SwAutoCompleteWord;

class SwFmtRefMark;
class SwFmtCol;
class SwNumRule;		// Numerierung
class SwNodeNum;		// Numerierung
class SwTxtFmtColl;
#ifdef USED
class SwGrfFmt;
class SwGrfFmtColl;
#endif
class SwGrfNode;
class SwFlyFrmFmt;

class SwFrmFmt;			// fuer GetTxtNodeFmts()
class SwCharFmt;
class SwExtTextInput;
class SwTxtNode;		// fuer IsNotMakeTxtNode
class SwRootFrm;		// fuer CTOR
class SwFmtINetFmt; 	// InsertURL

class SwTable;
class SwTextBlocks; 	// fuer GlossaryRW
class SwBlockExceptList;
class SwFmtFtn;
class SpellCheck;
class SwSection;
class SwSectionFmt;
class SwTOXMarks;
class SwTOXBase;
class SwTOXType;
class SwTableAutoFmt;
class SwPageDesc;
class SchMemChart;

class SwTxtINetFmt;
class SwSeqFldList;
class SwGlblDocContent;
class SwGlblDocContents;
class SwRedline;
class SwRedlineData;
class SwFtnInfo;
class SwEndNoteInfo;
class SwLineNumberInfo;
class SwAttrSet;
class SwAuthEntry;

//STRIP008 namespace com { namespace sun { namespace star { namespace uno {
//STRIP008 	template < class > class Sequence;
//STRIP008 }}}};


// Flags for GetScriptType - to define how handle weak - scripts (b.e.
// symbol characters):
// GETSCRIPT_WEAKTOAPPLANGSCRIPT:
// 		app language define the script type if only weak characters
// 		are selected and before the weak script no weak script if found
// 		(mostly the default for Get any attributes, etc)
// GETSCRIPT_WEAKTOAPPLANGSCRIPT:
// 		app language defines the script type if only weak characters
// 		are selected and before the weak script no weak script is found
// 		(mostly the default for Get any attributes, etc)
// GETSCRIPT_WEAKTOALL:
// 		all script flags are set.
// 		are selected and before the weak script no weak script if found
// 		(mostly the default for Get any attributes, etc)
#define GETSCRIPT_WEAKTOAPPLANGSCRIPT	0
#define GETSCRIPT_WEAKTOALL				1


#define GETSELTXT_PARABRK_TO_BLANK 		0
#define GETSELTXT_PARABRK_KEEP			1
#define GETSELTXT_PARABRK_TO_ONLYCR 	2

/****************************************************************
 *	zum Abfragen der INet-Attribute fuer den Navigator
 ****************************************************************/
struct SwGetINetAttr
{
    String sText;
    const SwTxtINetFmt& rINetAttr;

    SwGetINetAttr( const String& rTxt, const SwTxtINetFmt& rAttr )
        : sText( rTxt ), rINetAttr( rAttr )
    {}
};
SV_DECL_PTRARR_DEL( SwGetINetAttrs, SwGetINetAttr*, 0, 5 )

/****************************************************************
 *	Typen der Inhaltsformen
 ****************************************************************/
#define CNT_TXT	0x0001
#define CNT_GRF	0x0002
#define CNT_OLE	0x0010
/* Teste einen USHORT auf eine bestimmte Inhaltsform */
#define CNT_HasTxt(USH)	((USH)&CNT_TXT)
#define CNT_HasGrf(USH)	((USH)&CNT_GRF)
#define CNT_HasOLE(USH)	((USH)&CNT_OLE)

class SwEditShell: public SwCrsrShell
{
    static SvxSwAutoFmtFlags* pAutoFmtFlags;

    // fuer die privaten Methoden DelRange und die vom AutoCorrect
    friend void _InitCore();
    friend void _FinitCore();
    // fuer die PamCorrAbs/-Rel Methoden

    SfxPoolItem& _GetChrFmt( SfxPoolItem& ) const;

    /*
     * liefert einen Pointer auf einen SwGrfNode; dieser wird von
     * GetGraphic() und GetGraphicSize() verwendet.
     */



public:

#ifdef USED
    // GRF
    USHORT GetGrfFmtCollCount() const;
    SwGrfFmtColl& GetGrfFmtColl( USHORT nGrfFmtColl) const;
    void DelGrfFmtColl(USHORT nFmt);
    SwGrfFmtColl* GetCurGrfFmtColl() const;
    void SetGrfFmtColl(SwGrfFmtColl*);
    SwGrfFmtColl *MakeGrfFmtColl(const String &rFmtCollName,
        SwGrfFmtColl *pDerivedFrom = 0);
#endif

        // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        // sie noch nicht, dann erzuege sie
        // return das geforderte automatische  Format - Basis-Klasse !
        // returne die geforderte automatische Seiten-Vorlage
        // returne die geforderte automatische NumRule

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird

        // returne das geforderte automatische Format

    // Felder
    void Insert(SwField&);


    //check whether DB fields point to an available data source and returns it
    BOOL IsFieldDataSourceAvailable(String& rUsedDataSource) const;


    // Repeat
    // liefert die Id der letzten Repeatfaehigen Aktion zurueck
    // fuellt ggf. VARARR mit RedoIds

    // 0 letzte Aktion, sonst Aktionen bis zum Start der Klammerung nUndoId
    // mit KillPaMs, ClearMark
    // wiederholt
    // wiederholt
#ifdef USED
    // Aktionen klammern
#endif
    // fuer alle Sichten auf dieses Dokument
    void StartAllAction();
    void EndAllAction();

    //Damit Start-/EndActions aufgesetzt werden koennen.
    void CalcLayout();

    /* Anwenden der ViewOptions mit Start-/EndAction */
    inline void ApplyViewOptions( const SwViewOption &rOpt );


    // Is spelling active somewhere else?
    // Is text conversion active somewhere else?
    BOOL HasConvIter() const;

    // special insert: Insert a new text node just before or after a section or
    // table, if the cursor is positioned at the start/end of said
    // section/table. The purpose of the method is to allow users to inert text
    // at certain 'impossible' position, e.g. before a table at the document
    // start or between to sections.
    bool DoSpecialInsert();
    bool CanSpecialInsert() const;

    // Optimierung UI

    inline const SvxLinkManager& GetLinkManager() const;

    // Numberformatter vom Doc erfragen
    const SvNumberFormatter* GetNumberFormatter() const
    {	return ((SwEditShell*)this)->GetNumberFormatter(); 	}

    virtual ~SwEditShell();

private:
    // fuer METWARE:
    // es wird nicht kopiert und nicht zugewiesen
    SwEditShell(const SwEditShell &);
    const SwEditShell &operator=(const SwEditShell &);
};

inline void SwEditShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwCrsrShell::StartAction();
    ViewShell::ApplyViewOptions( rOpt );
    SwEditShell::EndAction();
}

inline const SvxLinkManager& SwEditShell::GetLinkManager() const
{	return ((SwEditShell*)this)->GetLinkManager();	}

/*
 * Klasse fuer den automatisierten Aufruf von Start- und
 * EndAction();
 */


/*
 * Klasse fuer den automatisierten Aufruf von Start- und
 * EndCrsrMove();
 */
class SwMvKontext {
    SwEditShell *pSh;
public:
    SwMvKontext(SwEditShell *pShell );
    ~SwMvKontext();
};

#define MV_KONTEXT(x)	SwMvKontext _aMvKontext_(x)



} //namespace binfilter
#endif
