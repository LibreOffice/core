/*************************************************************************
 *
 *  $RCSfile: frame.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ama $ $Date: 2001-09-14 14:08:42 $
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
#ifndef _FRAME_HXX
#define _FRAME_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#include "swtypes.hxx"  // fuer SwTwips
#include "swrect.hxx"
#include "calbck.hxx"   // fuer SwClient

typedef long Size::* SzPtr;
extern SzPtr pHeight;
extern SzPtr pWidth;
typedef long Point::* PtPtr;
extern PtPtr pX;
extern PtPtr pY;

//Liefern Memberpointer auf die jeweiligen Groessen.
#define pFIXPOS  ( bVarHeight ? pX : pY )
#define pFIXSIZE ( bVarHeight ? pWidth : pHeight )
#define pVARPOS  ( bVarHeight ? pY : pX )
#define pVARSIZE ( bVarHeight ? pHeight : pWidth )

class SwLayoutFrm;
class SwRootFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SdrObject;
class SwDrawContact;
class SwFtnFrm;
class SwFtnBossFrm;
class SwTabFrm;
class SwFlowFrm;
class SwCntntFrm;
class SfxPoolItem;
class SwAttrSet;
class ViewShell;
class Brush;
class Color;
class SwBorderAttrs;
class Sw3FrameIo;
class SwCache;
class SvxBrushItem;
class SwTxtFtn;
struct SwPosition;
struct SwCrsrMoveState;

//Jeder FrmTyp findet sich hier in einem Bit wieder.
//Die Bits muessen so gesetzt werden, dass mit einer Maskierung festgestellt
//werden kann was fuer ein FrmTyp eine Instanz ist _und_ von welchen Klassen
//sie abgeleitet ist.
//Der Frm hat in der Basisklasse einen Member der von den CToren der
//einzelnen Frms entsprechend gesetzt werden muss.
#define FRM_ROOT        0x0001
#define FRM_PAGE        0x0002
#define FRM_COLUMN      0x0004
#define FRM_HEADER      0x0008
#define FRM_FOOTER      0x0010
#define FRM_FTNCONT     0x0020
#define FRM_FTN         0x0040
#define FRM_BODY        0x0080
#define FRM_FLY         0x0100
#define FRM_SECTION     0x0200
#define FRM_UNUSED      0x0400
#define FRM_TAB         0x0800
#define FRM_ROW         0x1000
#define FRM_CELL        0x2000
#define FRM_TXT         0x4000
#define FRM_NOTXT       0x8000

//Fuer den internen Gebrauch ein paar gebraeuchliche Verknuepfungen.
#define FRM_LAYOUT      0x3FFF
#define FRM_CNTNT       0xC000
#define FRM_FTNBOSS     0x0006

        //Weils so schon ist das ganze als Bitfeld....
//0000 0000 0000 0001   ROOT
//0000 0000 0000 0010   PAGE
//0000 0000 0000 0100   COLUMN
//0000 0000 0000 1000   HEADER
//0000 0000 0001 0000   FOOTER
//0000 0000 0010 0000   FTNCONT
//0000 0000 0100 0000   FTN
//0000 0000 1000 0000   BODY
//0000 0001 0000 0000   FLY
//0000 0010 0000 0000   SECTION
//0000 0100 0000 0000   UNUSED
//0000 1000 0000 0000   TAB
//0001 0000 0000 0000   ROW
//0010 0000 0000 0000   CELL
//0100 0000 0000 0000   TXT
//1000 0000 0000 0000   NOTXT

#ifdef VERTICAL_LAYOUT
// The type of the frame is internal represented by the 4-bit value nType,
// which can expanded to the types above by shifting a bit (0x1 << nType)
// Here are the corresponding defines for the compressed representation:

#define FRMC_ROOT        0
#define FRMC_PAGE        1
#define FRMC_COLUMN      2
#define FRMC_HEADER      3
#define FRMC_FOOTER      4
#define FRMC_FTNCONT     5
#define FRMC_FTN         6
#define FRMC_BODY        7
#define FRMC_FLY         8
#define FRMC_SECTION     9
#define FRMC_UNUSED      10
#define FRMC_TAB         11
#define FRMC_ROW         12
#define FRMC_CELL        13
#define FRMC_TXT         14
#define FRMC_NOTXT       15

#define FRM_NEIGHBOUR   0x2004
#define FRM_NOTE_VERT   0x5a60

#define V_WIDTH SSize().*pDir1Sz
#define V_HEIGHT SSize().*pDir2Sz
#define V_X Pos().*pDir1Pt
#define V_Y Pos().*pDir2Pt

class SwFrm;
typedef long (SwFrm:: *SwFrmGet)() const;
typedef BOOL (SwFrm:: *SwFrmMax)( long );
typedef long (SwFrm:: *SwFrmDist)( long ) const;

struct SwRectFnCollection
{
    SwRectGet     fnGetTop;
    SwRectGet     fnGetBottom;
    SwRectGet     fnGetLeft;
    SwRectGet     fnGetRight;
    SwRectGet     fnGetWidth;
    SwRectGet     fnGetHeight;

    SwRectSet     fnSetTop;
    SwRectSet     fnSetBottom;
    SwRectSet     fnSetLeft;
    SwRectSet     fnSetRight;
    SwRectSet     fnSetWidth;
    SwRectSet     fnSetHeight;

    SwRectSet     fnSubTop;
    SwRectSet     fnAddBottom;
    SwRectSet     fnSubLeft;
    SwRectSet     fnAddRight;
    SwRectSet     fnAddWidth;
    SwRectSet     fnAddHeight;

    SwRectSet     fnSetPosX;
    SwRectSet     fnSetPosY;

    SwFrmGet      fnGetTopMargin;
    SwFrmGet      fnGetBottomMargin;
    SwFrmGet      fnGetLeftMargin;
    SwFrmGet      fnGetRightMargin;
    SwFrmGet      fnGetLimit;
    SwFrmMax      fnSetLimit;
    SwFrmDist     fnCheckLimit;
    SwFrmMax      fnOverStep;
};

typedef SwRectFnCollection* SwRectFn;
extern SwRectFn fnRectHori, fnRectVert;

#else

#define V_WIDTH SSize().*pWidth
#define V_HEIGHT SSize().*pHeight
#define V_X Pos().*pX
#define V_Y Pos().*pY

#define FRMC_ROOT        0x0001
#define FRMC_PAGE        0x0002
#define FRMC_COLUMN      0x0004
#define FRMC_HEADER      0x0008
#define FRMC_FOOTER      0x0010
#define FRMC_FTNCONT     0x0020
#define FRMC_FTN         0x0040
#define FRMC_BODY        0x0080
#define FRMC_FLY         0x0100
#define FRMC_SECTION     0x0200
#define FRMC_UNUSED      0x0400
#define FRMC_TAB         0x0800
#define FRMC_ROW         0x1000
#define FRMC_CELL        0x2000
#define FRMC_TXT         0x4000
#define FRMC_NOTXT       0x8000

#endif

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
                            //wird ein BOOL& uebergeben, dieser zeigt mit TRUE,
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

//Fuer GetNextLeaf/GetPrevLeaf.
enum MakePageType
{
    MAKEPAGE_NONE,      //Keine Seite bzw. Fussnote anlegen
    MAKEPAGE_APPEND,    //Nur ggf. Seite anhaengen
    MAKEPAGE_INSERT,    //Seite ggf. anhaengen oder einfuegen.
    MAKEPAGE_FTN,       //Fussnote ggf. einfuegen.
    MAKEPAGE_NOSECTION  // Don't create section frames
};

typedef SdrObject* SdrObjectPtr;
SV_DECL_PTRARR(SwDrawObjs,SdrObjectPtr,1,1);

class SwFrm: public SwClient
{
    //Der verkappte Frm
    friend class SwFlowFrm;
    friend class SwLayoutFrm;       // Sw3FrameIo: fuer pNext, pPrev
    friend class SwLooping;         // LoopControlling  (layouter.cxx)

        //Hebt die Lower waehrend eines Spaltenumbaus auf.
    friend SwFrm *SaveCntnt( SwLayoutFrm *, SwFrm* pStart = NULL );
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling );

    //Checkt ob sich beim MakePos die Pos des Frm aendert oder nicht
    //layact.cxx
    friend BOOL CheckPos( SwFrm *pFrm );

    //entfernt leere SwSectionFrms aus einer Kette
    friend SwFrm* SwClearDummies( SwFrm* pFrm );

        //Zum validieren eines unsinnig invalidierten in SwCntntFrm::MakeAll
    friend void ValidateSz( SwFrm *pFrm );
        // Implementiert in text/txtftn.cxx, verhindert Ftn-Oszillation
    friend void ValidateTxt( SwFrm *pFrm );

//  friend void CalcAnchorAndKeep( SwFlyFrm * );

    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );

    //Cache fuer (Umrandungs-)Attribute.
    static SwCache *pCache;

#ifndef PRODUCT
    //Nur zu Debugging Zwecken! eindeutige Nummerierung aller Frames,
    //verwaltet in den CToren.
    static USHORT nLastFrmId;
    const  USHORT nFrmId;
#endif

    SwLayoutFrm *pUpper;
    SwFrm       *pNext;
    SwFrm       *pPrev;

        //Schatten und Umrandung painten
    void PaintShadow( const SwRect&, SwRect&, const SwPageFrm *,
                      const SwBorderAttrs& ) const;
    SwFrm *_FindNext();
    SwFrm *_FindPrev();
    SwCntntFrm *_FindNextCnt();

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE & );
    SwFrm* _GetIndPrev();
    SwFrm* _GetIndNext();
#ifdef VERTICAL_LAYOUT
    void SetDirFlags( BOOL bVert );
#endif

    SwFrm( SwFrm & );       //Kopieren ist nicht erlaubt.
protected:
    SwDrawObjs *pDrawObjs;  //Hier haengen die DrawObjs, kann 0 sein

    SwRect  aFrm;   //Absolute Dokumentposition und groesse des Frm
    SwRect  aPrt;   //Position der PrtArea rel zum Frm und groesse der PrtArea

#ifdef VERTICAL_LAYOUT
    USHORT bFlag01:         1;
    USHORT bFlag02:         1;
    USHORT bFlag03:         1;
    USHORT bFlag04:         1;
    USHORT bFlag05:         1;
    USHORT bFlag06:         1;
    USHORT bInvalidR2L:     1;
    USHORT bDerivedR2L:     1;
    USHORT bRightToLeft:    1;
    USHORT bInvalidVert:    1;
    USHORT bDerivedVert:    1;
    USHORT bVertical:       1;
    USHORT nType:         4;  //Who am I?
#else
    USHORT  nType; // I am what I am.
#endif

    BOOL bValidPos:         1;
    BOOL bValidPrtArea:     1;
    BOOL bValidSize:        1;
    BOOL bValidLineNum:     1;
    BOOL bFixHeight:        1;
    BOOL bFixWidth:         1;
    BOOL bCompletePaint:    1;  //Frame wird ganz gepaintet wenn TRUE, auch
                                //wenn der Inhalt nur teilw. veraendert ist;
                                //Bei CntntFrms wird ausschliesslich wenn TRUE
                                //der Border (von Action) gemalt.
    BOOL bRetouche:         1;  //Der Frame ist fuer Retusche verantwortlich
                                //wenn TRUE.
public:
    BOOL bVarHeight:        1;  //Variable groesse ist die Hoehe wenn TRUE.
protected:
    BOOL bInfInvalid:       1;  //InfoFlags sind Invalid.
    BOOL bInfBody:          1;  //Frm steht im DokumentBody.
    BOOL bInfTab:           1;  //Frm steht in einer Tabelle.
    BOOL bInfFly:           1;  //Frm steht in einem Fly.
    BOOL bInfFtn:           1;  //Frm steht in einer Fussnote.
    BOOL bInfSct:           1;  //Frm steht in einem Bereich.
    BOOL bColLocked:        1;  //Grow/Shrink sperren bei spaltigen Section-
                                //oder Fly-Frames, wird im Format gesetzt

    void ColLock()      { bColLocked = TRUE; }
    void ColUnlock()    { bColLocked = FALSE; }

    SwPageFrm *InsertPage( SwPageFrm *pSibling, BOOL bFtn );
    void PrepareMake();
    void OptPrepareMake();
    void MakePos();
    virtual void MakeAll() = 0;
        //Adjustierung der Frames einer Seite
    SwTwips AdjustNeighbourhood( SwTwips nDiff, BOOL bTst = FALSE );


        //Aendern nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips ShrinkFrm( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE ) = 0;
    virtual SwTwips GrowFrm  ( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE ) = 0;

    SwModify        *GetDep()       { return pRegisteredIn; }
    const SwModify  *GetDep() const { return pRegisteredIn; }

    SwFrm( SwModify* );

    SwFrm( Sw3FrameIo&, SwLayoutFrm* );
public:
    TYPEINFO(); //Bereits in Basisklasse Client drin.

#ifdef VERTICAL_LAYOUT
    USHORT GetType() const { return 0x1 << nType; }
#else
    USHORT GetType() const { return nType; }
#endif

    static SwCache &GetCache()                { return *pCache; }
    static SwCache *GetCachePtr()             { return pCache;  }
    static void     SetCache( SwCache *pNew ) { pCache = pNew;  }

    virtual void Store( Sw3FrameIo& ) const;

        //Aendern die PrtArea-SSize und die FrmSize.
    SwTwips Shrink( SwTwips, const SzPtr,
                    BOOL bTst = FALSE, BOOL bInfo = FALSE );
    SwTwips Grow  ( SwTwips, const SzPtr,
                    BOOL bTst = FALSE, BOOL bInfo = FALSE );


    //Wir brauchen unterschiedliche Methoden (wg. Performance) fuer das
    //Einfuegenin den Layout Baum:

    //Einfuegen vor pBehind  oder am Ende der Kette unter pUpper
    void InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind );
    //Einfuegen hinter pBefore oder am Anfang der Kette unter pUpper
    void InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore );
    //Einfuegen vor pBehind oder am Ende der Kette, unter Beruecksichtigung
    //der Geschwister von pSct
    void InsertGroupBefore( SwFrm* pParent, SwFrm* pWhere, SwFrm* pSct );
    void Remove();

    //For internal use only; wer es anders macht wird
    //in einen Sack gesteckt und muss zwei Tage drin hocken bleiben.
    //Fuert Spezialbehandlung fuer _Get[Next|Prev]Leaf() durch (Tabellen).
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, BOOL bFwd );
    SwLayoutFrm *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrm *GetNextFtnLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetPrevLeaf   ( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevFtnLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevSctLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    const SwLayoutFrm *GetLeaf ( MakePageType eMakePage, BOOL bFwd,
                                 const SwFrm *pAnch ) const;
    BOOL WrongPageDesc( SwPageFrm* pNew );

    void AppendDrawObj( SwDrawContact *pObj );
    void RemoveDrawObj( SwDrawContact *pToRemove );

    //Arbeiten mit der Kette der FlyFrms
    void  AppendFly( SwFlyFrm *pNew );
    void  RemoveFly( SwFlyFrm *pToRemove );
    const SwDrawObjs *GetDrawObjs() const { return pDrawObjs; }
          SwDrawObjs *GetDrawObjs()       { return pDrawObjs; }
    void  CalcFlys( BOOL bInvaPosOnly );

    virtual void PaintBorder( const SwRect&, const SwPageFrm *pPage,
                              const SwBorderAttrs & ) const;
    void PaintBaBo( const SwRect&, const SwPageFrm *pPage = 0,
                    const BOOL bLowerBorder = FALSE ) const;
    void PaintBackground( const SwRect&, const SwPageFrm *pPage,
                          const SwBorderAttrs &,
                          const BOOL bLowerMode = FALSE,
                          const BOOL bLowerBorder = FALSE ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrm*,
                          const Color *pColor ) const;

    //Retouche, nicht im Bereich des uebergebenen Rect!
    void Retouche( const SwPageFrm *pPage, const SwRect &rRect ) const;

    BOOL GetBackgroundBrush( const SvxBrushItem*& rpBrush,
                             const Color*& rpColor,
                             SwRect &rOrigRect,
                             BOOL bLowerMode ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    inline BOOL IsCompletePaint() const { return bCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    inline BOOL IsRetouche() const { return bRetouche; }

    void SetInfFlags();                 //Setzen der InfoFlags
    inline void InvalidateInfFlags() { bInfInvalid = TRUE; }
    inline BOOL IsInDocBody() const;    //Benutzen die InfoFlags.
    inline BOOL IsInFtn() const;        //ggf. werden die Flags ermittelt.
    inline BOOL IsInTab() const;
    inline BOOL IsInFly() const;
    inline BOOL IsInSct() const;
#ifdef VERTICAL_LAYOUT
    inline BOOL IsVertical() const;
    inline BOOL GetVerticalFlag() const;
    inline void SetVertical( BOOL bNew ){ bVertical = bNew ? 1 : 0; }
    inline void SetDerivedVert( BOOL bNew ){ bDerivedVert = bNew ? 1 : 0; }
    inline void SetInvalidVert( BOOL bNew) { bInvalidVert = bNew ? 1 : 0; }
    inline BOOL IsRightToLeft() const;
    inline void SetRightToLeft( BOOL bNew ){ bRightToLeft = bNew ? 1 : 0; }
    inline void SetDerivedR2L( BOOL bNew ) { bDerivedR2L  = bNew ? 1 : 0; }
    inline void SetInvalidR2L( BOOL bNew ) { bInvalidR2L  = bNew ? 1 : 0; }
    void CheckVertical();
#endif
    BOOL IsMoveable() const;

    //Ist es fuer den (Txt)Frm in der aktuellen Umgebung erlaubt eine
    //Fussnote einzufuegen (nicht z.B. in wiederholten TabellenHeadlines).
    BOOL IsFtnAllowed() const;

    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );
    virtual void  Format( const SwBorderAttrs *pAttrs = 0 );

#ifdef VERTICAL_LAYOUT
    virtual void  CheckDirection( BOOL bVert );
#endif

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;
          SwAttrSet *GetAttrSet();
    void             GetAttrSet( SwAttrSet* );

    BOOL HasFixSize( const SzPtr ) const;

    //Kann 0 liefern, pruefen auch ob die Shell zum richtigen Dokument
    //gehoert. Impl in frmsh.hxx
    inline ViewShell *GetShell() const;

    //Prueft alle Seiten ab der Uebergebenen und korrigiert ggf.
    static void CheckPageDescs( SwPageFrm *pStart, BOOL bNotifyFields = TRUE );

        //Koennen 0 liefern, einmal const einmal nicht
    SwFrm               *GetNext()  { return pNext; }
    SwFrm               *GetPrev()  { return pPrev; }
    SwLayoutFrm         *GetUpper() { return pUpper; }
    SwRootFrm           *FindRootFrm();
    SwPageFrm           *FindPageFrm();
    SwFrm               *FindColFrm();
    SwFtnBossFrm        *FindFtnBossFrm( BOOL bFootnotes = FALSE );
    SwTabFrm            *ImplFindTabFrm();
    SwFtnFrm            *ImplFindFtnFrm();
    SwFlyFrm            *ImplFindFlyFrm();
    SwSectionFrm        *ImplFindSctFrm();
    SwSectionFrm        *ImplFindTopSctFrm();
    SwFrm               *FindFooterOrHeader();
    SwFrm               *GetLower();
    const SwFrm         *GetNext()  const { return pNext; }
    const SwFrm         *GetPrev()  const { return pPrev; }
    const SwLayoutFrm   *GetUpper() const { return pUpper; }
    inline SwTabFrm     *FindTabFrm();
    inline SwFtnFrm     *FindFtnFrm();
    inline SwFlyFrm     *FindFlyFrm();
    inline SwSectionFrm *FindSctFrm();
    inline SwSectionFrm *FindTopSctFrm();
    inline SwFrm        *FindNext();
    inline SwCntntFrm   *FindNextCnt();
    inline SwFrm        *FindPrev();
    inline const SwPageFrm *FindPageFrm() const;
    inline const SwRootFrm *FindRootFrm() const;
    inline const SwFtnBossFrm *FindFtnBossFrm( BOOL bFtn = FALSE ) const;
    inline const SwFrm     *FindColFrm() const;
    inline const SwFrm     *FindFooterOrHeader() const;
    inline const SwTabFrm  *FindTabFrm() const;
    inline const SwFtnFrm  *FindFtnFrm() const;
    inline const SwFlyFrm  *FindFlyFrm() const;
    inline const SwSectionFrm *FindSctFrm() const;
    inline const SwSectionFrm *FindTopSctFrm() const;
    inline const SwFrm     *FindNext() const;
    inline const SwCntntFrm *FindNextCnt() const;
    inline const SwFrm     *FindPrev() const;
           const SwFrm     *GetLower()  const;

    SwFrm* GetIndPrev()
        { return ( pPrev || !IsInSct() ) ? pPrev : _GetIndPrev(); }
    const SwFrm* GetIndPrev() const { return ((SwFrm*)this)->GetIndPrev(); }
    SwFrm* GetIndNext()
        { return ( pNext || !IsInSct() ) ? pNext : _GetIndNext(); }
    const SwFrm* GetIndNext() const { return ((SwFrm*)this)->GetIndNext(); }

    USHORT GetPhyPageNum() const;   //Seitennummer ohne Offset
    USHORT GetVirtPageNum() const;  //Seitenummer mit Offset
    BOOL OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    BOOL WannaRightPage() const;

    const  SwLayoutFrm *GetPrevLayoutLeaf() const;
    const  SwLayoutFrm *GetNextLayoutLeaf() const;
    inline SwLayoutFrm *GetPrevLayoutLeaf();
    inline SwLayoutFrm *GetNextLayoutLeaf();

    inline void Calc() const;       //Hier wird ggf. 'Formatiert'
    inline void OptCalc() const;    //Hier wird zur Optimierung davon ausgegangen,
                                    //das die Vorgaenger bereits formatiert sind.

    inline Point   GetRelPos() const;
    const  SwRect &Frm() const { return aFrm; }
    const  SwRect &Prt() const { return aPrt; }

    // The PaintArea is the area, where content may be displayed.
    // The margin of the page or the space between columns belongs to her.
    const SwRect PaintArea() const;
    // The UnionFrm is the union of frm- and prt-area, normally identical
    // to the frm-area except the case of negative prt-margins.
    const SwRect UnionFrm( BOOL bBorder = FALSE ) const;

    //Der Zugriff auf die Member wird hier ausnahmsweiste gestattet,
    //dies soll aber nicht dazu dienen die Werte wahllos zu veraendern;
    //es ist nur die einzige Moeglichkeit die Compilerprobleme zu umgehen
    //(gleiche Methode mal public mal protected).
    SwRect &Frm() { return aFrm; }
    SwRect &Prt() { return aPrt; }

    virtual void ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) = 0;

    void ValidateLineNum() { bValidLineNum = TRUE; }

    BOOL GetValidPosFlag()    const { return bValidPos; }
    BOOL GetValidPrtAreaFlag()const { return bValidPrtArea; }
    BOOL GetValidSizeFlag()   const { return bValidSize; }
    BOOL GetValidLineNumFlag()const { return bValidLineNum; }
    BOOL IsValid() const { return bValidPos && bValidSize && bValidPrtArea; }

        //Invalideren nur den Frm
    void _InvalidateSize()      { bValidSize = FALSE; }
    void _InvalidatePrt()       { bValidPrtArea = FALSE; }
    void _InvalidatePos()       { bValidPos = FALSE; }
    void _InvalidateLineNum()   { bValidLineNum = FALSE; }
    void _InvalidateAll()       { bValidSize = bValidPrtArea = bValidPos = FALSE; }

    //Benachrichtigen gleich die Seite mit.
    inline void InvalidateSize();
    inline void InvalidatePrt();
    inline void InvalidatePos();
    inline void InvalidateLineNum();
    inline void InvalidateAll();
    void ImplInvalidateSize();
    void ImplInvalidatePrt();
    void ImplInvalidatePos();
    void ImplInvalidateLineNum();

    inline void InvalidateNextPos( BOOL bNoFtn = FALSE );
    void ImplInvalidateNextPos( BOOL bNoFtn = FALSE );
    void InvalidatePage( const SwPageFrm *pPage = 0 ) const;

    virtual BOOL    GetCrsrOfst( SwPosition *, Point&,
                                 const SwCrsrMoveState* = 0 ) const;
    virtual BOOL    GetCharRect( SwRect &, const SwPosition&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual void    Paint( const SwRect& ) const;

    // der "kurze Dienstweg" zwischen den Frames und der Formatierung.
    // Wer den void* falsch Casted ist selbst schuld!
    // Auf jedenfall muss der void* auf 0 geprueft werden.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, BOOL bNotify = TRUE );

    //TRUE wenn's die richtige Klasse ist, FALSE sonst
    inline BOOL IsLayoutFrm() const;
    inline BOOL IsRootFrm() const;
    inline BOOL IsPageFrm() const;
    inline BOOL IsColumnFrm() const;
    inline BOOL IsFtnBossFrm() const; // Fussnotenbosse sind PageFrms und ColumnFrms
    inline BOOL IsHeaderFrm() const;
    inline BOOL IsFooterFrm() const;
    inline BOOL IsFtnContFrm() const;
    inline BOOL IsFtnFrm() const;
    inline BOOL IsBodyFrm() const;
    inline BOOL IsColBodyFrm() const;   // in layfrm.hxx implementiert, BodyFrm unterhalb ColumnFrm
    inline BOOL IsPageBodyFrm() const;  // in layfrm.hxx implementiert, BodyFrm unterhalb PageFrm
    inline BOOL IsFlyFrm() const;
    inline BOOL IsSctFrm() const;
    inline BOOL IsTabFrm() const;
    inline BOOL IsRowFrm() const;
    inline BOOL IsCellFrm() const;
    inline BOOL IsCntntFrm() const;
    inline BOOL IsTxtFrm() const;
    inline BOOL IsNoTxtFrm() const;
    inline BOOL IsFlowFrm() const;      //Frms deren PrtArea von den Nachbarn
                                        //abhaengen und die halt im Inhaltsfluss
                                        //stehen.
    inline BOOL IsRetoucheFrm() const;  //Frms die Retouchefaehig sind bzw. die
                                        //u.U. hinter sich Retouchieren muessen.

    void PrepareCrsr();                 //Die CrsrShell darf.

    //Ist der Frm (bzw. die Section in der er steht) geschuetzt?
    //Auch Fly in Fly in ... und Fussnoten
    BOOL IsProtected() const;

    BOOL IsColLocked()  const { return bColLocked; }

    virtual ~SwFrm();

#ifdef VERTICAL_LAYOUT
    // No inline cause we need the function pointers
    long GetTopMargin() const;
    long GetBottomMargin() const;
    long GetLeftMargin() const;
    long GetRightMargin() const;
    long GetPrtLeft() const;
    long GetPrtBottom() const;
    BOOL SetMinLeft( long );
    BOOL SetMaxBottom( long );
    long GetLeftDistance( long ) const;
    long GetBottomDistance( long ) const;
    BOOL OverStepLeft( long );
    BOOL OverStepBottom( long );
#endif

#ifndef PRODUCT
    inline USHORT GetFrmId() const { return nFrmId; }
    inline USHORT GetLastFrmId() const { return nLastFrmId; }
#endif
};

inline BOOL SwFrm::IsInDocBody() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfBody;
}
inline BOOL SwFrm::IsInFtn() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFtn;
}
inline BOOL SwFrm::IsInTab() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfTab;
}
inline BOOL SwFrm::IsInFly() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFly;
}
inline BOOL SwFrm::IsInSct() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfSct;
}
#ifdef VERTICAL_LAYOUT
BOOL SwFrm::IsVertical() const
{
    if( bInvalidVert )
        ((SwFrm*)this)->SetDirFlags( TRUE );
    return bVertical != 0;
}
BOOL SwFrm::GetVerticalFlag() const
{
    return bVertical != 0;
}
inline BOOL SwFrm::IsRightToLeft() const
{
    if( bInvalidR2L )
        ((SwFrm*)this)->SetDirFlags( FALSE );
    return bRightToLeft != 0;
}
#endif

inline void SwFrm::SetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = TRUE;
}
inline void SwFrm::ResetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = FALSE;
}

inline void SwFrm::SetRetouche() const
{
    ((SwFrm*)this)->bRetouche = TRUE;
}
inline void SwFrm::ResetRetouche() const
{
    ((SwFrm*)this)->bRetouche = FALSE;
}

inline SwLayoutFrm *SwFrm::GetPrevLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetPrevLayoutLeaf();
}
inline SwLayoutFrm *SwFrm::GetNextLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetNextLayoutLeaf();
}

inline void SwFrm::InvalidateSize()
{
    if ( bValidSize )
        ImplInvalidateSize();
}
inline void SwFrm::InvalidatePrt()
{
    if ( bValidPrtArea )
        ImplInvalidatePrt();
}
inline void SwFrm::InvalidatePos()
{
    if ( bValidPos )
        ImplInvalidatePos();
}
inline void SwFrm::InvalidateLineNum()
{
    if ( bValidLineNum )
        ImplInvalidateLineNum();
}

inline void SwFrm::InvalidateAll()
{
    if ( bValidPrtArea && bValidSize && bValidPos  )
        ImplInvalidatePos();
    bValidPrtArea = bValidSize = bValidPos = FALSE;
}

inline void SwFrm::InvalidateNextPos( BOOL bNoFtn )
{
    if ( pNext && !pNext->IsSctFrm() )
        pNext->InvalidatePos();
#ifndef C30 // vielleicht geht es ja bei C40 ?
    else
        ImplInvalidateNextPos( bNoFtn );
#else
    if ( !pNext )
        ImplInvalidateNextPos( bNoFtn );
#endif
}

inline void SwFrm::Calc() const
{
    if ( !bValidPos || !bValidPrtArea || !bValidSize )
        ((SwFrm*)this)->PrepareMake();
}
inline void SwFrm::OptCalc() const
{
    if ( !bValidPos || !bValidPrtArea || !bValidSize )
        ((SwFrm*)this)->OptPrepareMake();
}

inline Point SwFrm::GetRelPos() const
{
    Point aRet( aFrm.Pos() );
        //hier wird gecasted, weil die Klasse SwLayoutFrm nur vorward-
        //declariert ist.
    aRet -= ((SwFrm*)GetUpper())->Prt().Pos();
    aRet -= ((SwFrm*)GetUpper())->Frm().Pos();
    return aRet;
}

inline const SwPageFrm *SwFrm::FindPageFrm() const
{
    return ((SwFrm*)this)->FindPageFrm();
}
inline const SwRootFrm *SwFrm::FindRootFrm() const
{
    return ((SwFrm*)this)->FindRootFrm();
}
inline const SwFrm *SwFrm::FindColFrm() const
{
    return ((SwFrm*)this)->FindColFrm();
}
inline const SwFrm *SwFrm::FindFooterOrHeader() const
{
    return ((SwFrm*)this)->FindFooterOrHeader();
}
inline SwTabFrm *SwFrm::FindTabFrm()
{
    return IsInTab() ? ImplFindTabFrm() : 0;
}
inline const SwFtnBossFrm *SwFrm::FindFtnBossFrm( BOOL bFtn ) const
{
    return ((SwFrm*)this)->FindFtnBossFrm( bFtn );
}
inline SwFtnFrm *SwFrm::FindFtnFrm()
{
    return IsInFtn() ? ImplFindFtnFrm() : 0;
}
inline SwFlyFrm *SwFrm::FindFlyFrm()
{
    return IsInFly() ? ImplFindFlyFrm() : 0;
}
inline SwSectionFrm *SwFrm::FindSctFrm()
{
    return IsInSct() ? ImplFindSctFrm() : 0;
}

inline SwSectionFrm *SwFrm::FindTopSctFrm()
{
    return IsInSct() ? ImplFindTopSctFrm() : 0;
}

inline const SwTabFrm *SwFrm::FindTabFrm() const
{
    return IsInTab() ? ((SwFrm*)this)->ImplFindTabFrm() : 0;
}
inline const SwFtnFrm *SwFrm::FindFtnFrm() const
{
    return IsInFtn() ? ((SwFrm*)this)->ImplFindFtnFrm() : 0;
}
inline const SwFlyFrm *SwFrm::FindFlyFrm() const
{
    return IsInFly() ? ((SwFrm*)this)->ImplFindFlyFrm() : 0;
}
inline const SwSectionFrm *SwFrm::FindSctFrm() const
{
    return IsInSct() ? ((SwFrm*)this)->ImplFindSctFrm() : 0;
}
inline const SwSectionFrm *SwFrm::FindTopSctFrm() const
{
    return IsInSct() ? ((SwFrm*)this)->ImplFindTopSctFrm() : 0;
}
inline SwFrm *SwFrm::FindNext()
{
    if ( pNext )
        return pNext;
    else
        return _FindNext();
}
inline const SwFrm *SwFrm::FindNext() const
{
    if ( pNext )
        return pNext;
    else
        return ((SwFrm*)this)->_FindNext();
}
inline SwCntntFrm *SwFrm::FindNextCnt()
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return _FindNextCnt();
}
inline const SwCntntFrm *SwFrm::FindNextCnt() const
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return ((SwFrm*)this)->_FindNextCnt();
}
inline SwFrm *SwFrm::FindPrev()
{
    if ( pPrev && !pPrev->IsSctFrm() )
        return pPrev;
    else
        return _FindPrev();
}
inline const SwFrm *SwFrm::FindPrev() const
{
    if ( pPrev && !pPrev->IsSctFrm() )
        return pPrev;
    else
        return ((SwFrm*)this)->_FindPrev();
}

inline BOOL SwFrm::IsLayoutFrm() const
{
    return GetType() & FRM_LAYOUT ? TRUE : FALSE;
}
inline BOOL SwFrm::IsRootFrm() const
{
    return nType == FRMC_ROOT;
}
inline BOOL SwFrm::IsPageFrm() const
{
    return nType == FRMC_PAGE;
}
inline BOOL SwFrm::IsColumnFrm() const
{
    return nType == FRMC_COLUMN;
}
inline BOOL SwFrm::IsFtnBossFrm() const
{
    return GetType() & FRM_FTNBOSS ? TRUE : FALSE;
}
inline BOOL SwFrm::IsHeaderFrm() const
{
    return nType == FRMC_HEADER;
}
inline BOOL SwFrm::IsFooterFrm() const
{
    return nType == FRMC_FOOTER;
}
inline BOOL SwFrm::IsFtnContFrm() const
{
    return nType == FRMC_FTNCONT;
}
inline BOOL SwFrm::IsFtnFrm() const
{
    return nType == FRMC_FTN;
}
inline BOOL SwFrm::IsBodyFrm() const
{
    return nType == FRMC_BODY;
}
inline BOOL SwFrm::IsFlyFrm() const
{
    return nType == FRMC_FLY;
}
inline BOOL SwFrm::IsSctFrm() const
{
    return nType == FRMC_SECTION;
}
inline BOOL SwFrm::IsTabFrm() const
{
    return nType == FRMC_TAB;
}
inline BOOL SwFrm::IsRowFrm() const
{
    return nType == FRMC_ROW;
}
inline BOOL SwFrm::IsCellFrm() const
{
    return nType == FRMC_CELL;
}
inline BOOL SwFrm::IsCntntFrm() const
{
    return GetType() & FRM_CNTNT ? TRUE : FALSE;
}
inline BOOL SwFrm::IsTxtFrm() const
{
    return nType == FRMC_TXT;
}
inline BOOL SwFrm::IsNoTxtFrm() const
{
    return nType == FRMC_NOTXT;
}
inline BOOL SwFrm::IsFlowFrm() const
{
    return GetType() & 0xCA00 ? TRUE : FALSE;   //TabFrm, CntntFrm, SectionFrm
}
inline BOOL SwFrm::IsRetoucheFrm() const
{
    return GetType() & 0xCA40 ? TRUE : FALSE;   //TabFrm, CntntFrm, SectionFrm, Ftnfrm
}

#endif
