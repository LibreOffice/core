/*************************************************************************
 *
 *  $RCSfile: frame.hxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:05:04 $
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

#define SZPTR
#define PHEIGHT
#define PWIDTH
#define BFIXHEIGHT bFixSize
#define PTPTR PointPtr
#define SIZEPTR SizePtr
typedef long Size::* SizePtr;
typedef long Point::* PointPtr;


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
class SwRowFrm;
class SwFlowFrm;
class SwCntntFrm;
class SfxPoolItem;
class SwAttrSet;
class ViewShell;
class Color;
class SwBorderAttrs;
class SwCache;
class SvxBrushItem;
class SwTxtFtn;
struct SwPosition;
struct SwCrsrMoveState;
// OD 20.05.2003 #108784#
class SwDrawVirtObj;

// --> OD 2004-07-06 #i28701#
class SwSortedObjs;
class SwAnchoredObject;
// <--

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
#ifdef ACCESSIBLE_LAYOUT
#define FRM_ACCESSIBLE (FRM_HEADER|FRM_FOOTER|FRM_FTN|FRM_TXT|FRM_ROOT|FRM_FLY|FRM_TAB|FRM_CELL|FRM_PAGE)
#endif

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
#define FRM_HEADFOOT    0x0018
#define FRM_BODYFTNC    0x00a0

class SwFrm;
typedef long (SwFrm:: *SwFrmGet)() const;
typedef BOOL (SwFrm:: *SwFrmMax)( long );
typedef void (SwFrm:: *SwFrmMakePos)( const SwFrm*, const SwFrm*, BOOL );
typedef long (*SwOperator)( long, long );
typedef void (SwFrm:: *SwFrmSet)( long, long );

struct SwRectFnCollection
{
    SwRectGet     fnGetTop;
    SwRectGet     fnGetBottom;
    SwRectGet     fnGetLeft;
    SwRectGet     fnGetRight;
    SwRectGet     fnGetWidth;
    SwRectGet     fnGetHeight;
    SwRectPoint   fnGetPos;
    SwRectSize    fnGetSize;

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
    SwFrmSet      fnSetXMargins;
    SwFrmSet      fnSetYMargins;
    SwFrmGet      fnGetPrtTop;
    SwFrmGet      fnGetPrtBottom;
    SwFrmGet      fnGetPrtLeft;
    SwFrmGet      fnGetPrtRight;
    SwRectDist    fnTopDist;
    SwRectDist    fnBottomDist;
    SwRectDist    fnLeftDist;
    SwRectDist    fnRightDist;
    SwFrmMax      fnSetLimit;
    SwRectMax     fnOverStep;

    SwRectSetPos  fnSetPos;
    SwFrmMakePos  fnMakePos;
    SwOperator    fnXDiff;
    SwOperator    fnYDiff;
    SwOperator    fnXInc;
    SwOperator    fnYInc;

    SwRectSetTwice fnSetLeftAndWidth;
    SwRectSetTwice fnSetTopAndHeight;
};

typedef SwRectFnCollection* SwRectFn;
extern SwRectFn fnRectHori, fnRectVert, fnRectB2T, fnRectVL2R;

#define SWRECTFN( pFrm )    sal_Bool bVert = pFrm->IsVertical(); \
                            sal_Bool bRev = pFrm->IsReverse(); \
                            SwRectFn fnRect = bVert ? \
                                ( bRev ? fnRectVL2R : fnRectVert ): \
                                ( bRev ? fnRectB2T : fnRectHori );
#define SWRECTFNX( pFrm )   sal_Bool bVertX = pFrm->IsVertical(); \
                            sal_Bool bRevX = pFrm->IsReverse(); \
                            SwRectFn fnRectX = bVertX ? \
                                ( bRevX ? fnRectVL2R : fnRectVert ): \
                                ( bRevX ? fnRectB2T : fnRectHori );
#define SWREFRESHFN( pFrm ) { if( bVert != pFrm->IsVertical() || \
                                  bRev  != pFrm->IsReverse() ) \
                                bVert = pFrm->IsVertical(); \
                                bRev = pFrm->IsReverse(); \
                                fnRect = bVert ? \
                                    ( bRev ? fnRectVL2R : fnRectVert ): \
                                    ( bRev ? fnRectB2T : fnRectHori ); }
#define SWRECTFN2( pFrm )   sal_Bool bVert = pFrm->IsVertical(); \
                            sal_Bool bNeighb = pFrm->IsNeighbourFrm(); \
                            SwRectFn fnRect = bVert == bNeighb ? \
                                fnRectHori : fnRectVert;
#define POS_DIFF( aFrm1, aFrm2 ) \
            ( (aFrm1.*fnRect->fnGetTop)() != (aFrm2.*fnRect->fnGetTop)() || \
            (aFrm1.*fnRect->fnGetLeft)() != (aFrm2.*fnRect->fnGetLeft)() )


//Fuer GetNextLeaf/GetPrevLeaf.
enum MakePageType
{
    MAKEPAGE_NONE,      //Keine Seite bzw. Fussnote anlegen
    MAKEPAGE_APPEND,    //Nur ggf. Seite anhaengen
    MAKEPAGE_INSERT,    //Seite ggf. anhaengen oder einfuegen.
    MAKEPAGE_FTN,       //Fussnote ggf. einfuegen.
    MAKEPAGE_NOSECTION  // Don't create section frames
};

// OD 2004-05-06 #i28701# - replaced by new class <SwSortedObjs>
//typedef SdrObject* SdrObjectPtr;
//SV_DECL_PTRARR(SwDrawObjs,SdrObjectPtr,1,1);

class SwFrm: public SwClient
{
    //Der verkappte Frm
    friend class SwFlowFrm;
    friend class SwLayoutFrm;       // Sw3FrameIo: fuer pNext, pPrev
    friend class SwLooping;         // LoopControlling  (layouter.cxx)

        //Hebt die Lower waehrend eines Spaltenumbaus auf.
    friend SwFrm *SaveCntnt( SwLayoutFrm *, SwFrm* pStart = NULL );
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling, bool bGrow );

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

    SwFrm *_FindNext();
    SwFrm *_FindPrev();
    SwCntntFrm *_FindNextCnt();

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE & );
    SwFrm* _GetIndPrev();
    SwFrm* _GetIndNext();
    void SetDirFlags( BOOL bVert );

    SwFrm( SwFrm & );       //Kopieren ist nicht erlaubt.

    const SwLayoutFrm* ImplGetNextLayoutLeaf( bool bFwd ) const;

protected:
    SwSortedObjs* pDrawObjs;    //Hier haengen die DrawObjs, kann 0 sein

    SwRect  aFrm;   //Absolute Dokumentposition und groesse des Frm
    SwRect  aPrt;   //Position der PrtArea rel zum Frm und groesse der PrtArea

    USHORT bFlag01:         1;
    USHORT bFlag02:         1;
    USHORT bFlag03:         1;
    USHORT bFlag04:         1;
    USHORT bFlag05:         1;
    USHORT bReverse:        1; // Next line above/at the right side instead
                               // under/at the left side of the previous line.
    USHORT bInvalidR2L:     1;
    USHORT bDerivedR2L:     1;
    USHORT bRightToLeft:    1;
    USHORT bInvalidVert:    1;
    USHORT bDerivedVert:    1;
    USHORT bVertical:       1;
    USHORT nType:         4;  //Who am I?

    BOOL bValidPos:         1;
    BOOL bValidPrtArea:     1;
    BOOL bValidSize:        1;
    BOOL bValidLineNum:     1;
    BOOL bFixSize:          1;
    BOOL bUnUsed1:          1;
    BOOL bCompletePaint:    1;  //Frame wird ganz gepaintet wenn TRUE, auch
                                //wenn der Inhalt nur teilw. veraendert ist;
                                //Bei CntntFrms wird ausschliesslich wenn TRUE
                                //der Border (von Action) gemalt.
    BOOL bRetouche:         1;  //Der Frame ist fuer Retusche verantwortlich
                                //wenn TRUE.
public:
    BOOL bUnUsed2:          1;
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
    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE ) = 0;
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE ) = 0;

    SwModify        *GetDep()       { return pRegisteredIn; }
    const SwModify  *GetDep() const { return pRegisteredIn; }

    SwFrm( SwModify* );

    void CheckDir( UINT16 nDir, BOOL bVert, BOOL bOnlyBiDi, BOOL bBrowse );

    /** enumeration for the different invalidations

        OD 2004-05-19 #i28701#

        @author OD
    */
    enum InvalidationType
    {
        INVALID_SIZE, INVALID_PRTAREA, INVALID_POS, INVALID_LINENUM, INVALID_ALL
    };

    /** method to determine, if an invalidation is allowed.

        OD 2004-05-19 #i28701

        @author OD
    */
    virtual bool _InvalidationAllowed( const InvalidationType _nInvalid ) const;

    /** method to perform additional actions on an invalidation

        OD 2004-05-19 #i28701#
        Method has *only* to contain actions, which has to be performed on
        *every* assignment of the corresponding flag to <FALSE>.

        @author OD
    */
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid );

        //Schatten und Umrandung painten
    void PaintShadow( const SwRect&, SwRect&, const SwPageFrm *,
                      const SwBorderAttrs& ) const;

public:
    TYPEINFO(); //Bereits in Basisklasse Client drin.

    USHORT GetType() const { return 0x1 << nType; }

    static SwCache &GetCache()                { return *pCache; }
    static SwCache *GetCachePtr()             { return pCache;  }
    static void     SetCache( SwCache *pNew ) { pCache = pNew;  }

        //Aendern die PrtArea-SSize und die FrmSize.
    SwTwips Shrink( SwTwips, SZPTR
                    BOOL bTst = FALSE, BOOL bInfo = FALSE );
    SwTwips Grow  ( SwTwips, SZPTR
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
    SwLayoutFrm *GetNextCellLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetPrevLeaf   ( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevFtnLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevSctLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevCellLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    const SwLayoutFrm *GetLeaf ( MakePageType eMakePage, BOOL bFwd,
                                 const SwFrm *pAnch ) const;

    BOOL WrongPageDesc( SwPageFrm* pNew );

    // --> OD 2004-07-02 #i28701# - new methods to append/remove drawing objects
    void AppendDrawObj( SwAnchoredObject& _rNewObj );
    void RemoveDrawObj( SwAnchoredObject& _rToRemoveObj );
    // <--

    //Arbeiten mit der Kette der FlyFrms
    void  AppendFly( SwFlyFrm *pNew );
    void  RemoveFly( SwFlyFrm *pToRemove );
    const SwSortedObjs *GetDrawObjs() const { return pDrawObjs; }
          SwSortedObjs *GetDrawObjs()         { return pDrawObjs; }
    // --> OD 2004-07-01 #i28701# - change purpose of method and adjust its name
    void InvalidateObjs( const bool _bInvaPosOnly,
                         const bool _bNoInvaOfAsCharAnchoredObjs = true );

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

    // If frame is inside a split table row, this function returns
    // the corresponding row frame in the follow table.
   const SwRowFrm* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
   const SwRowFrm* IsInFollowFlowRow() const;

    inline BOOL IsReverse() const { return bReverse; }
    inline void SetReverse( BOOL bNew ){ bReverse = bNew ? 1 : 0; }
    inline BOOL IsVertical() const;
    inline BOOL GetVerticalFlag() const;
    inline void SetVertical( BOOL bNew ){ bVertical = bNew ? 1 : 0; }
    inline void SetDerivedVert( BOOL bNew ){ bDerivedVert = bNew ? 1 : 0; }
    inline void SetInvalidVert( BOOL bNew) { bInvalidVert = bNew ? 1 : 0; }
    inline BOOL IsRightToLeft() const;
    inline BOOL GetRightToLeftFlag() const;
    inline void SetRightToLeft( BOOL bNew ){ bRightToLeft = bNew ? 1 : 0; }
    inline void SetDerivedR2L( BOOL bNew ) { bDerivedR2L  = bNew ? 1 : 0; }
    inline void SetInvalidR2L( BOOL bNew ) { bInvalidR2L  = bNew ? 1 : 0; }
    void CheckDirChange();
    // returns upper left frame position for LTR and
    // upper right frame position for Asian / RTL frames
    Point   GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        OD 08.08.2003 #110978#
        method replaced 'old' method <BOOL IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrm == 0L), the movability in the actual
        environment (<this->GetUpper()) is checked.

        @author OD

        @param _pLayoutFrm
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0L ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
//    BOOL IsMoveable() const;
    bool IsMoveable( const SwLayoutFrm* _pLayoutFrm = 0L ) const;

    //Ist es fuer den (Txt)Frm in der aktuellen Umgebung erlaubt eine
    //Fussnote einzufuegen (nicht z.B. in wiederholten TabellenHeadlines).
    BOOL IsFtnAllowed() const;

    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );
    virtual void  Format( const SwBorderAttrs *pAttrs = 0 );

    virtual void  CheckDirection( BOOL bVert );

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;
          SwAttrSet *GetAttrSet();
    void             GetAttrSet( SwAttrSet* );

    inline BOOL HasFixSize() const { return bFixSize; }
    inline void SetFixSize( BOOL bNew ) { bFixSize = bNew; }

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


    inline const  SwLayoutFrm *GetPrevLayoutLeaf() const;
    inline const  SwLayoutFrm *GetNextLayoutLeaf() const;
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
    // OD 2004-05-19 #i28701# - add call to method <_ActionOnInvalidation(..)>
    // for all invalidation methods.
    // OD 2004-05-19 #i28701# - use method <_InvalidationAllowed(..)> to
    // decide, if invalidation will to be performed or not.
    void _InvalidateSize()
    {
        if ( _InvalidationAllowed( INVALID_SIZE ) )
        {
            bValidSize = FALSE;
            _ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void _InvalidatePrt()
    {
        if ( _InvalidationAllowed( INVALID_PRTAREA ) )
        {
            bValidPrtArea = FALSE;
            _ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void _InvalidatePos()
    {
        if ( _InvalidationAllowed( INVALID_POS ) )
        {
            bValidPos = FALSE;
            _ActionOnInvalidation( INVALID_POS );
        }
    }
    void _InvalidateLineNum()
    {
        if ( _InvalidationAllowed( INVALID_LINENUM ) )
        {
            bValidLineNum = FALSE;
            _ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void _InvalidateAll()
    {
        if ( _InvalidationAllowed( INVALID_ALL ) )
        {
            bValidSize = bValidPrtArea = bValidPos = FALSE;
            _ActionOnInvalidation( INVALID_ALL );
        }
    }

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

    /** method to invalidate printing area of next frame

        OD 09.01.2004 #i11859#

        @author OD
    */
    void InvalidateNextPrtArea();

    void InvalidatePage( const SwPageFrm *pPage = 0 ) const;

    virtual BOOL    GetCrsrOfst( SwPosition *, Point&,
                                 SwCrsrMoveState* = 0 ) const;
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
#ifdef ACCESSIBLE_LAYOUT
    inline BOOL IsAccessibleFrm() const;
#endif

    void PrepareCrsr();                 //Die CrsrShell darf.

    //Ist der Frm (bzw. die Section in der er steht) geschuetzt?
    //Auch Fly in Fly in ... und Fussnoten
    BOOL IsProtected() const;

    BOOL IsColLocked()  const { return bColLocked; }

    virtual ~SwFrm();

    // No inline cause we need the function pointers
    long GetTopMargin() const;
    long GetBottomMargin() const;
    long GetLeftMargin() const;
    long GetRightMargin() const;
    void SetTopBottomMargins( long, long );
    void SetBottomTopMargins( long, long );
    void SetLeftRightMargins( long, long );
    void SetRightLeftMargins( long, long );
    void SetLeftAndWidth( long nLeft, long nWidth );
    void SetTopAndHeight( long nTop, long nHeight );
    void SetRightAndWidth( long nRight, long nWidth );
    void SetBottomAndHeight( long nBottom, long nHeight );
    long GetPrtLeft() const;
    long GetPrtBottom() const;
    long GetPrtRight() const;
    long GetPrtTop() const;
    BOOL SetMinLeft( long );
    BOOL SetMaxBottom( long );
    BOOL SetMaxRight( long );
    BOOL SetMinTop( long );
    void MakeBelowPos( const SwFrm*, const SwFrm*, BOOL );
    void MakeUpperPos( const SwFrm*, const SwFrm*, BOOL );
    void MakeLeftPos( const SwFrm*, const SwFrm*, BOOL );
    void MakeRightPos( const SwFrm*, const SwFrm*, BOOL );
    inline BOOL SwFrm::IsNeighbourFrm() const
        { return GetType() & FRM_NEIGHBOUR ? TRUE : FALSE; }

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
BOOL SwFrm::GetRightToLeftFlag() const
{
    return bRightToLeft != 0;
}

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

inline SwLayoutFrm *SwFrm::GetNextLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetNextLayoutLeaf();
}

inline SwLayoutFrm *SwFrm::GetPrevLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetPrevLayoutLeaf();
}

inline const SwLayoutFrm *SwFrm::GetNextLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( true );
}

inline const SwLayoutFrm *SwFrm::GetPrevLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( false );
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
    if ( _InvalidationAllowed( INVALID_ALL ) )
    {
        if ( bValidPrtArea && bValidSize && bValidPos  )
            ImplInvalidatePos();
        bValidPrtArea = bValidSize = bValidPos = FALSE;

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_ALL );
    }
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
#ifdef ACCESSIBLE_LAYOUT
inline BOOL SwFrm::IsAccessibleFrm() const
{
    return GetType() & FRM_ACCESSIBLE ? TRUE : FALSE;
}
#endif

#endif
