/*************************************************************************
 *
 *  $RCSfile: wrtrtf.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-13 16:56:38 $
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

#ifndef _WRTRTF_HXX
#define _WRTRTF_HXX

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _WRT_FN_HXX
#include <wrt_fn.hxx>
#endif

// einige Forward Deklarationen
class Color;
class Font;
class SvPtrarr;
class RTFColorTbl;
class SwFmt;
class SwFmtColl;
class SwFlyFrmFmt;
class SwPosFlyFrms;
class SwPageDesc;
class SwTableNode;
class SwTxtFmtColl;
class SwNumRule;
class SwNumRuleTbl;
class SwNodeNum;
class DateTime;
class RTFEndPosLst;

extern SwAttrFnTab aRTFAttrFnTab;
extern SwNodeFnTab aRTFNodeFnTab;

// the text encoding for the export
#ifdef MAC
#define DEF_ENCODING        RTL_TEXTENCODING_APPLE_ROMAN
#else
#define DEF_ENCODING        RTL_TEXTENCODING_MS_1252
#endif


// der RTF-Writer

class SwRTFWriter : public Writer
{
    friend class RTFEndPosLst;

    SvPtrarr* pFontRemoveLst;
    RTFColorTbl* pColTbl;
    SwPosFlyFrms* pFlyPos;          // Pointer auf die aktuelle "FlyFrmTabelle"
    RTFEndPosLst* pCurEndPosLst;
    const SfxItemSet* pAttrSet;     // akt. Format/Collection vom Node
                                    // fuer den Zugriff auf einige Attribute
                                    // z.B. Font-Size, LR-Space,..

    USHORT nAktFlyPos;              // Index auf das naechste "FlyFrmFmt"
    void OutRTFColorTab();
    void OutRTFFontTab();
    void OutRTFStyleTab();
    void OutRTFListTab();

    void MakeHeader();
    void OutDocInfoStat();
    void OutInfoDateTime( const DateTime&, const sal_Char* );
    void CheckEndNodeForSection( const SwNode& rNd );


public:
    // --- public Member --------------------------------------------------

    SwFlyFrmFmt* pFlyFmt;               // liegt der Node in einem FlyFrame,
                                        // ist das Format gesetzt, sonst 0
    const SwPageDesc* pAktPageDesc;     // aktuell gesetzter PageDesc.
    USHORT nBkmkTabPos;             // akt. Position in der Bookmark-Tabelle
    USHORT nCurScript;                  // actual scripttype

#if defined(MAC) || defined(UNX)
    static const sal_Char sNewLine;                 // nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];    // \015\012
#endif


    BOOL bFirstLine : 1;            // wird die 1. Zeile ausgegeben ?
    BOOL bOutFmtAttr : 1;           // TRUE: beim Schreiben eines Formates
                                    // existierte mindestens ein Attribut
    BOOL bRTFFlySyntax : 1;         // gebe nur original RTFSyntax aus
                                    // (nur fuer die fliegenden Rahmen)
    BOOL bOutPageDesc: 1;           // gebe einen PageDescriptor aus
    BOOL bOutPageDescTbl: 1;        // gebe die PageDescriptor-Tabelle aus
    BOOL bOutTable : 1;             // gebe eine Tabelle aus
    BOOL bTxtAttr : 1;              // werden TextAttribute ausgegeben ?
    BOOL bWriteHelpFmt : 1;         // schreibe Win-RTF-HelpFileFmt
    BOOL bOutStyleTab : 1;          // gebe die StyleSheet-Tabelle aus
    BOOL bOutPageAttr : 1;          // PageDescAttribut ausgeben?
    BOOL bAutoAttrSet : 1;          // TRUE:  pAttrSet ist harte Attributierung
                                    // FALSE: pAttrSet ist vom Format/Collection
    BOOL bOutOutlineOnly : 1;       // TRUE: nur Gliederungs-Absaetze schreiben
    BOOL bOutListNumTxt : 1;        // TRUE: der ListNumText wird ausgegeben
    BOOL bOutLeftHeadFoot : 1;      // gebe vom PageDesc. den linkten
                                    // Header/Footer aus
    BOOL bOutSection : 1;           // TRUE: Section PageDesc ausgeben
    BOOL bIgnoreNextPgBreak : 1;    // TRUE: naechsten PageDesc/Break ignorieren
    BOOL bAssociated : 1;           // use associated tokens

    // --- public Methoden ------------------------------------------------

    SwRTFWriter( const String& rFilterName );
    virtual ~SwRTFWriter();
    virtual ULONG WriteStream();

    void Out_SwDoc( SwPaM* );       // schreibe den makierten Bereich

        // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    void OutFlyFrm();
    void OutRTFFlyFrms( const SwFlyFrmFmt& );
        // gebe alle an der Position stehenden Bookmarks aus
    void OutBookmarks( xub_StrLen nCntntPos );
        // gebe die PageDesc-Daten im normalen RTF-Format aus
    void OutRTFPageDescription( const SwPageDesc&, BOOL , BOOL );
    BOOL OutBreaks( const SfxItemSet& rSet );

        // gebe die PageDescriptoren aus
    USHORT GetId( const Color& ) const;
    USHORT GetId( const SvxFontItem& ) const;
    USHORT GetId( const Font& ) const;
    USHORT GetId( const SwTxtFmtColl& ) const;
    USHORT GetId( const SwCharFmt& ) const;
    USHORT GetId( const SwNumRuleItem& rItem ) const;

    void OutPageDesc();

    void OutListNum( const SwTxtNode& rNd );

    // fuer RTFSaveData
    SwPaM* GetEndPaM()              { return pOrigPam; }
    void SetEndPaM( SwPaM* pPam )   { pOrigPam = pPam; }

    const SfxPoolItem& GetItem( USHORT nWhich ) const;

    const SfxItemSet* GetAttrSet() const    { return pAttrSet; }
    void SetAttrSet( const SfxItemSet* p )  { pAttrSet = p; }

    const RTFEndPosLst* GetEndPosLst() const { return pCurEndPosLst; }

    void SetAssociatedFlag( BOOL b )        { bAssociated = b; }
    BOOL IsAssociatedFlag() const           { return bAssociated; }

    void SetCurrScriptType( USHORT n )      { nCurScript = n; }
    USHORT GetCurrScriptType() const        { return nCurScript; }
};


// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
// Mit den beiden USHORTs im CTOR wird ein neuer PaM erzeugt und auf
// die Position im Dokument gesetzt.
// Im Destructor werden alle Daten wieder restauriert und der angelegte
// Pam wieder geloescht.

struct RTFSaveData
{
    SwRTFWriter& rWrt;
    SwPaM* pOldPam, *pOldEnd;
    SwFlyFrmFmt* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;
    const SfxItemSet* pOldAttrSet;          // akt. Attribute vom Node

    BOOL bOldWriteAll : 1;
    BOOL bOldOutTable : 1;
    BOOL bOldOutPageAttr : 1;
    BOOL bOldAutoAttrSet : 1;
    BOOL bOldOutSection : 1;

    RTFSaveData( SwRTFWriter&, ULONG nStt, ULONG nEnd );
    ~RTFSaveData();
};


// einige Funktions-Deklarationen
Writer& OutRTF_AsByteString( Writer& rWrt, const String& rStr );
Writer& OutRTF_SwFmt( Writer& rWrt, const SwFmt& );
Writer& OutRTF_SwTblNode( Writer& , SwTableNode & );
Writer& OutRTF_SwSectionNode( Writer& , SwSectionNode & );


// Augabe von RTF-Bitmaps (steht im File "wrtpict.cxx")
//struct SvxRTFPictureType;
//class Bitmap;
//USHORT WriteRTFPict( const SwPictureType&, Bitmap&, SvStream& );

// Ausagbe von Footer-/Headers
Writer& OutRTF_SwFmtHeader( Writer& , const SfxPoolItem& );
Writer& OutRTF_SwFmtFooter( Writer& , const SfxPoolItem& );

// Kommentar und zusaetzlichen String ausgeben
SvStream& OutComment( Writer& rWrt, const sal_Char* pStr );
    // zusaetzlich das bOutFmtAttr-Flag manipulieren
SvStream& OutComment( Writer& rWrt, const sal_Char* pStr, BOOL bSetFlag );

#endif  //  _WRTRTF_HXX


