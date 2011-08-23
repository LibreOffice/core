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
#ifndef _NDTXT_HXX
#define _NDTXT_HXX

#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _NDHINTS_HXX
#include <ndhints.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
namespace com { namespace sun { namespace star { namespace uno {
    template < class > class Sequence;
}}}}
class OutputDevice;
namespace utl {
    class TransliterationWrapper;
}//STRIP008 ;
namespace binfilter {



class SwNumRules;
class SwTxtFmtColl;
class SwCntntFrm;
class SwTxtFld; 		 // Fuer GetTxtFld()
class SwAttrSet;


struct SwSpellArgs;             // for Spell(), splargs.hxx
struct SwConversionArgs;        // for Convert(), splargs.hxx
class SwInterHyphInfo;          // for Hyphenate(), splargs.hxx
class SwWrongList;      // fuer OnlineSpelling
class SwNodeNum;


// Konstanten fuer das Text-Insert:
#define INS_DEFAULT         0x0000 // keine Extras
#define INS_EMPTYEXPAND     0x0001 // leere Hints beim Einfuegen aufspannen
#define INS_NOHINTEXPAND    0x0002 // Hints an der InsPos nicht aufspannen

//STRIP008 namespace com { namespace sun { namespace star { namespace uno {
//STRIP008     template < class > class Sequence;
//STRIP008 }}}}

// --------------------
// SwTxtNode
// --------------------
class SwTxtNode: public SwCntntNode
{
    // fuer das Erzeugen des ersten TextNode
    friend class SwDoc; 		// CTOR und AppendTxtNode()
    friend class SwNodes;
    friend class SwTxtFrm;
    friend void SwpHints::Insert( SwTxtAttr*, SwTxtNode&, USHORT );

    //Kann 0 sein, nur dann nicht 0 wenn harte Attribute drin stehen.
    //Also niemals direkt zugreifen!
    SwpHints	*pSwpHints;
    SwWrongList	*pWrong;	// Rechtschreibfehler nach Online-Spelling
    SwNodeNum	*pNdNum;	// Numerierung fuer diesen Absatz
    SwNodeNum	*pNdOutl;	// Outline fuer diesen Absatz
    XubString	aText;

    SwTxtNode( const SwNodeIndex &rWhere, SwTxtFmtColl *pTxtColl,
                SwAttrSet* pAutoAttr = 0 );

    // Kopiert die Attribute an nStart nach pDest.
    void CopyAttr( SwTxtNode *pDest, const xub_StrLen nStart, const xub_StrLen nOldPos);

    SwTxtNode* _MakeNewTxtNode( const SwNodeIndex&, BOOL bNext = TRUE,
                                BOOL bChgFollow = TRUE );

    void	_Cut( SwTxtNode *pDest, const SwIndex &rDestStart,
                  const SwIndex &rStart, xub_StrLen nLen, BOOL bUpdate = TRUE );

    SwTxtAttr* MakeTxtAttr( const SfxPoolItem& rNew, xub_StrLen nStt, xub_StrLen nEnd,
                            BOOL bPool = TRUE );

    // Verlagere alles umfassende harte Attribute in den AttrSet des Absatzes

    // lege den spz. AttrSet an
    virtual void NewAttrSet( SwAttrPool& );

    void Replace0xFF( XubString& rTxt, xub_StrLen& rTxtStt,
                        xub_StrLen nEndPos, BOOL bExpandFlds ) const;

public:
    const String& GetTxt() const { return aText; }

    // Zugriff auf SwpHints
    inline 		 SwpHints &GetSwpHints();
    inline const SwpHints &GetSwpHints() const;
    inline 		 SwpHints *GetpSwpHints() { return pSwpHints; }
    inline const SwpHints *GetpSwpHints() const { return pSwpHints; }
    void 		 SetWrong( SwWrongList *pNew );
    inline 		 SwWrongList *GetWrong() { return pWrong; }
    inline const SwWrongList *GetWrong() const { return pWrong; }
    inline BOOL  HasHints() const { return pSwpHints ? TRUE : FALSE; }
    inline 		 SwpHints &GetOrCreateSwpHints();

    virtual ~SwTxtNode();

    virtual xub_StrLen Len() const;

    // steht in itratr

    /*
     * Einfuegen anderer Datentypen durch Erzeugen eines
     * temporaeren Strings.
     */
    SwTxtNode&	Insert( xub_Unicode c, const SwIndex &rIdx );
    SwTxtNode&	Insert( const XubString &rStr, const SwIndex &rIdx,
                        const USHORT nMode = INS_DEFAULT );

    SwTxtNode&	Erase( const SwIndex &rIdx, xub_StrLen nCount = STRING_LEN,
                       const USHORT nMode = INS_DEFAULT );

    // Aktionen auf Attributen
    // loesche alle TextAttribute die als Attribut im Set vorhanden sind
    // (Set-Pointer != 0 ) oder alle deren Which-Wert mit nWhich mappen
    // oder wenn Which = 0, alle.
    void	RstAttr( const SwIndex &rIdx, xub_StrLen nLen, USHORT nWhich = 0,
                    const SfxItemSet* pSet = 0, BOOL bInclRefToxMark = FALSE );

    // loesche das Text-Attribut (muss beim Pool abgemeldet werden!)
    void 	DestroyAttr( SwTxtAttr* pAttr );

    // loesche alle Attribute aus dem SwpHintsArray.

    // uebernehme den Pointer auf das Text-Attribut
    BOOL	Insert( SwTxtAttr *pAttr, USHORT nMode = 0 );
    // lege ein neues TextAttribut an und fuege es ins SwpHints-Array ein
    // returne den neuen Pointer (oder 0 bei Fehlern)!
    SwTxtAttr* Insert( const SfxPoolItem& rAttr,
                        xub_StrLen nStt, xub_StrLen nEnd, USHORT nMode = 0 );

    // setze diese Attribute am TextNode. Wird der gesamte Bereich umspannt,
    // dann setze sie nur im AutoAttrSet (SwCntntNode:: SetAttr)
    BOOL SetAttr( const SfxItemSet& rSet,
                    xub_StrLen nStt, xub_StrLen nEnd, USHORT nMode = 0 );
    // erfrage die Attribute vom TextNode ueber den Bereich
    BOOL GetAttr( SfxItemSet& rSet, xub_StrLen nStt, xub_StrLen nEnd,
                    BOOL bOnlyTxtAttr  = FALSE,
                    BOOL bGetFromChrFmt = TRUE ) const;

    // uebertrage Attribute eines AttrSets ( AutoFmt ) in das SwpHintsArray
    void FmtToTxtAttr( SwTxtNode* pNd );

    // loeschen eines einzelnen Attributes (fuer SwUndoAttr)
    // ( nur das Attribut loeschen, dass mit Which,Start und End oder
    //   mit pTxtHint identisch ist (es gibt nur ein gueltiges))
    // 	AUSNAHME: ToxMarks !!!
    void 	Delete( USHORT nTxtWhich, xub_StrLen nStart, xub_StrLen nEnd = 0 );
    void 	Delete( SwTxtAttr * pTxtAttr, BOOL bThisOnly = FALSE );

    // Aktionen auf Text und Attributen
    void	Copy(SwTxtNode *pDest, const SwIndex &rStart, USHORT nLen);
    void	Copy(SwTxtNode *pDest, const SwIndex &rDestStart,
                const SwIndex &rStart, xub_StrLen nLen);
    void	Cut(SwTxtNode *pDest, const SwIndex &rStart, xub_StrLen nLen);
    inline void	Cut(SwTxtNode *pDest, const SwIndex &rDestStart,
                    const SwIndex &rStart, xub_StrLen nLen);
    // ersetze im String an Position nIdx das Zeichen
    void Replace( const SwIndex& rStart, xub_Unicode cCh );

    // virtuelle Methoden aus dem CntntNode
    virtual SwCntntFrm *MakeFrm();
    virtual SwCntntNode *SplitNode( const SwPosition & );
    virtual SwCntntNode *JoinNext();
    virtual SwCntntNode *JoinPrev();

    SwCntntNode *AppendNode( const SwPosition & );

    // setze ggf. das DontExpand-Flag an INet bzw. Zeichenvorlagen

    // gebe das vorgegebene Attribut, welches an der TextPosition (rIdx)
    // gesetzt ist zurueck. Gibt es keines, returne 0-Pointer
    // gesetzt heisst: Start <= rIdx < End
    SwTxtAttr *GetTxtAttr( const SwIndex& rIdx, USHORT nWhichHt,
                           BOOL bExpand = FALSE ) const;

    // Diese Methode liefert nur Textattribute auf der Position nIdx
    // zurueck, die kein EndIdx besitzen und denselben Which besitzen.
    // Ueblicherweise steht an dieser Position ein CH_TXTATR.
    // Bei RES_TXTATR_END entfaellt die Pruefung auf den Which-Wert.
    SwTxtAttr *GetTxtAttr( const xub_StrLen nIdx,
                           const USHORT nWhichHt = RES_TXTATR_END ) const;

    SwTxtFld  *GetTxtFld( const SwIndex& rIdx )
        { return (SwTxtFld *)GetTxtAttr( rIdx, RES_TXTATR_FIELD ); }

    // Aktuelles Wort zurueckliefern
    USHORT Convert( SwConversionArgs & );

    inline SwTxtFmtColl *GetTxtColl() const;
    virtual SwFmtColl *ChgFmtColl( SwFmtColl* );
    void _ChgTxtCollUpdateNum( const SwTxtFmtColl* pOld,
                                const SwTxtFmtColl* pNew );

    // kopiere die Collection mit allen Autoformaten zum Dest-Node
    // dieser kann auch in einem anderen Dokument stehen!
    // (Methode steht im ndcopy.cxx!!)

    // Numerierung
    const SwNodeNum* UpdateNum( const SwNodeNum& );
    const SwNumRule *GetNumRule() const;
    const SwNodeNum* GetNum() const				{ return pNdNum; }
    // OutlineNumerierung
    const SwNodeNum* UpdateOutlineNum( const SwNodeNum& );
    const SwNodeNum* GetOutlineNum() const		{ return pNdOutl; }
    void NumRuleChgd();					// Frames benachrichtigen
    XubString GetNumString() const;		// returnt Outline oder Num - String
    long GetLeftMarginWithNum( BOOL bTxtLeft = FALSE ) const;
    BOOL GetFirstLineOfsWithNum( short& rFirstOffset ) const;

#ifdef VERTICAL_LAYOUT
    USHORT GetLang( const xub_StrLen nBegin, const xub_StrLen nLen = 0,
                    USHORT nScript = 0 ) const;
#else
    USHORT GetLang( const xub_StrLen nBegin, const xub_StrLen nLen = 0) const;
#endif
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    // interaktive Trennung: wir finden den TxtFrm und rufen sein CalcHyph

    // Liefert einen String mit expandierten Feldern zurueck
    //   opt. die Kapitel/Nummer-String zurueck
    XubString GetExpandTxt( const xub_StrLen nIdx = 0,
                         const xub_StrLen nLen = STRING_LEN,
                         const BOOL bWithNum = FALSE ) const;
    //Liefert fuer die Initalfunktion tatsaechliche Anzahl der Initialzeichen
    //bei nWishLen == 0 die des ersten Wortes
    USHORT GetDropLen( USHORT nWishLen) const;

    // Berechnung des Visible-Flags
    inline BOOL CalcVisibleFlag()
        { if(pSwpHints) return pSwpHints->CalcVisibleFlag(); return FALSE; }
    // Setzen des CalcVisible-Flags
    inline void SetCalcVisible(){ if(pSwpHints) pSwpHints->SetCalcVisible(); }

    // Ist der Absatz sichtbar
    inline BOOL IsVisible() const
        { return pSwpHints ? pSwpHints->IsVisible() : TRUE; }

    // Besitzt der Absatz Fussnoten?
    inline BOOL HasFtn() const {return pSwpHints ? pSwpHints->HasFtn() : FALSE;}

    inline SwTxtAttr* MakeTmpTxtAttr( const SfxPoolItem& rNew )
        { return MakeTxtAttr( rNew, 0, 0, FALSE ); }

    TYPEINFO();	// fuer rtti

    // fuers Umhaengen der TxtFmtCollections (Outline-Nummerierung!!)
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
//	virtual BOOL GetInfo( SfxPoolItem& ) const;

    // aus SwIndexReg
    virtual void Update( const SwIndex & aPos, USHORT xub_StrLen,
                        BOOL bNegativ = FALSE );


    // change text to Upper/Lower/Hiragana/Katagana/...


    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtNode)
};

//-----------------------------------------------------------------------------

inline SwpHints	&SwTxtNode::GetSwpHints()
{
    ASSERT_ID( pSwpHints, ERR_NOHINTS);
    return *pSwpHints;
}
inline const SwpHints &SwTxtNode::GetSwpHints() const
{
    ASSERT_ID( pSwpHints, ERR_NOHINTS);
    return *pSwpHints;
}

inline SwpHints& SwTxtNode::GetOrCreateSwpHints()
{
    if( !pSwpHints )
        pSwpHints = new SwpHints;
    return *pSwpHints;
}

inline SwTxtFmtColl* SwTxtNode::GetTxtColl() const
{
    return (SwTxtFmtColl*)GetRegisteredIn();
}

// fuer den IBM-Compiler nicht inlinen wg. 42876
#ifndef ICC
// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline       SwTxtNode   *SwNode::GetTxtNode()
{
     return ND_TEXTNODE == nNodeType ? (SwTxtNode*)this : 0;
}
inline const SwTxtNode   *SwNode::GetTxtNode() const
{
     return ND_TEXTNODE == nNodeType ? (const SwTxtNode*)this : 0;
}
#endif
#endif

inline void	SwTxtNode::Cut(SwTxtNode *pDest, const SwIndex &rDestStart,
                            const SwIndex &rStart, xub_StrLen nLen)
{
    _Cut( pDest, rDestStart, rStart, nLen, TRUE );
}


} //namespace binfilter
#endif
