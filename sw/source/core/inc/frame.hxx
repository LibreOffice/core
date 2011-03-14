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
#ifndef SW_FRAME_HXX
#define SW_FRAME_HXX

#include <svl/svarray.hxx>
#include <editeng/borderline.hxx>
#include "swtypes.hxx"  // fuer SwTwips
#include "swrect.hxx"
#include "calbck.hxx"   // fuer SwClient

#if OSL_DEBUG_LEVEL > 1
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#endif

class SwLayoutFrm;
class SwRootFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SdrObject;
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
class SwSelectionList;
struct SwPosition;
struct SwCrsrMoveState;
class SwPrintData;

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
#define FRM_ACCESSIBLE (FRM_HEADER|FRM_FOOTER|FRM_FTN|FRM_TXT|FRM_ROOT|FRM_FLY|FRM_TAB|FRM_CELL|FRM_PAGE)

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
#define FRM_NOTE_VERT   0x7a60
#define FRM_HEADFOOT    0x0018
#define FRM_BODYFTNC    0x00a0

class SwFrm;
typedef long (SwFrm:: *SwFrmGet)() const;
typedef sal_Bool (SwFrm:: *SwFrmMax)( long );
typedef void (SwFrm:: *SwFrmMakePos)( const SwFrm*, const SwFrm*, sal_Bool );
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
/*
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
*/

//Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
extern SwRectFn fnRectHori, fnRectVert, fnRectB2T, fnRectVL2R, fnRectVertL2R;
#define SWRECTFN( pFrm )    sal_Bool bVert = pFrm->IsVertical(); \
                            sal_Bool bRev = pFrm->IsReverse(); \
                            sal_Bool bVertL2R = pFrm->IsVertLR(); \
                            SwRectFn fnRect = bVert ? \
                                ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRev ? fnRectB2T : fnRectHori );
#define SWRECTFNX( pFrm )   sal_Bool bVertX = pFrm->IsVertical(); \
                            sal_Bool bRevX = pFrm->IsReverse(); \
                            sal_Bool bVertL2RX = pFrm->IsVertLR(); \
                            SwRectFn fnRectX = bVertX ? \
                                ( bRevX ? fnRectVL2R : ( bVertL2RX ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRevX ? fnRectB2T : fnRectHori );
#define SWREFRESHFN( pFrm ) { if( bVert != pFrm->IsVertical() || \
                                  bRev  != pFrm->IsReverse() ) \
                                bVert = pFrm->IsVertical(); \
                                bRev = pFrm->IsReverse(); \
                                bVertL2R = pFrm->IsVertLR(); \
                                fnRect = bVert ? \
                                    ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                    ( bRev ? fnRectB2T : fnRectHori ); }
#define SWRECTFN2( pFrm )   sal_Bool bVert = pFrm->IsVertical(); \
                sal_Bool bVertL2R = pFrm->IsVertLR(); \
                            sal_Bool bNeighb = pFrm->IsNeighbourFrm(); \
                            SwRectFn fnRect = bVert == bNeighb ? \
                                fnRectHori : ( bVertL2R ? fnRectVertL2R : fnRectVert );
//End of SCMS
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

#if OSL_DEBUG_LEVEL > 1
    //entfernt leere SwSectionFrms aus einer Kette
    friend SwFrm* SwClearDummies( SwFrm* pFrm );
#endif

        //Zum validieren eines unsinnig invalidierten in SwCntntFrm::MakeAll
    friend void ValidateSz( SwFrm *pFrm );
        // Implementiert in text/txtftn.cxx, verhindert Ftn-Oszillation
    friend void ValidateTxt( SwFrm *pFrm );

//  friend void CalcAnchorAndKeep( SwFlyFrm * );

    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );

    //Cache fuer (Umrandungs-)Attribute.
    static SwCache *pCache;

    // --> OD 2006-05-10 #i65250#
    // frame ID is now in general available - used for layout loop control
    static sal_uInt32 mnLastFrmId;
    const  sal_uInt32 mnFrmId;
    // <--

    SwLayoutFrm *pUpper;
    SwFrm       *pNext;
    SwFrm       *pPrev;

    SwFrm *_FindNext();
    SwFrm *_FindPrev();

    /** method to determine next content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        OD 2005-11-30 #i27138# - adding documentation:
        Travelling downwards through the layout to determine the next content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        OD 2005-11-30 #i27138# - adding parameter <_bInSameFtn>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @author OD

        @param _bInSameFtn
        input parameter - boolean indicating, that the found next content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwCntntFrm*
        pointer to the found next content frame. It's NULL, if none exists.
    */
    SwCntntFrm* _FindNextCnt( const bool _bInSameFtn = false );

    /** method to determine previous content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        OD 2005-11-30 #i27138#
        Travelling upwards through the layout to determine the previous content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        OD 2005-11-30 #i27138# - adding parameter <_bInSameFtn>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @author OD

        @param _bInSameFtn
        input parameter - boolean indicating, that the found previous content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwCntntFrm*
        pointer to the found previous content frame. It's NULL, if none exists.
    */
    SwCntntFrm* _FindPrevCnt( const bool _bInSameFtn = false );


    void _UpdateAttrFrm( SfxPoolItem*, SfxPoolItem*, sal_uInt8 & );
    SwFrm* _GetIndNext();
    void SetDirFlags( sal_Bool bVert );

    SwFrm( SwFrm & );       //Kopieren ist nicht erlaubt.

    const SwLayoutFrm* ImplGetNextLayoutLeaf( bool bFwd ) const;

protected:
    SwSortedObjs* pDrawObjs;    //Hier haengen die DrawObjs, kann 0 sein

    SwRect  aFrm;   //Absolute Dokumentposition und groesse des Frm
    SwRect  aPrt;   //Position der PrtArea rel zum Frm und groesse der PrtArea

    sal_uInt16 bFlag01:         1;
    sal_uInt16 bFlag02:         1;
    sal_uInt16 bFlag03:         1;
    sal_uInt16 bFlag04:         1;
    sal_uInt16 bFlag05:         1;
    sal_uInt16 bReverse:        1; // Next line above/at the right side instead
                               // under/at the left side of the previous line.
    sal_uInt16 bInvalidR2L:     1;
    sal_uInt16 bDerivedR2L:     1;
    sal_uInt16 bRightToLeft:    1;
    sal_uInt16 bInvalidVert:    1;
    sal_uInt16 bDerivedVert:    1;
    sal_uInt16 bVertical:       1;
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    sal_uInt16 bVertLR:         1;
    //End of SCMS
    sal_uInt16 nType:         4;  //Who am I?

    sal_Bool bValidPos:         1;
    sal_Bool bValidPrtArea:     1;
    sal_Bool bValidSize:        1;
    sal_Bool bValidLineNum:     1;
    sal_Bool bFixSize:          1;
    sal_Bool bUnUsed1:          1;
    sal_Bool bCompletePaint:    1;  //Frame wird ganz gepaintet wenn sal_True, auch
                                //wenn der Inhalt nur teilw. veraendert ist;
                                //Bei CntntFrms wird ausschliesslich wenn sal_True
                                //der Border (von Action) gemalt.
    sal_Bool bRetouche:         1;  //Der Frame ist fuer Retusche verantwortlich
                                //wenn sal_True.
public:
    sal_Bool bUnUsed2:          1;
protected:
    sal_Bool bInfInvalid:       1;  //InfoFlags sind Invalid.
    sal_Bool bInfBody:          1;  //Frm steht im DokumentBody.
    sal_Bool bInfTab:           1;  //Frm steht in einer Tabelle.
    sal_Bool bInfFly:           1;  //Frm steht in einem Fly.
    sal_Bool bInfFtn:           1;  //Frm steht in einer Fussnote.
    sal_Bool bInfSct:           1;  //Frm steht in einem Bereich.
    sal_Bool bColLocked:        1;  //Grow/Shrink sperren bei spaltigen Section-
                                //oder Fly-Frames, wird im Format gesetzt

    void ColLock()      { bColLocked = sal_True; }
    void ColUnlock()    { bColLocked = sal_False; }

    SwPageFrm *InsertPage( SwPageFrm *pSibling, sal_Bool bFtn );
    void PrepareMake();
    void OptPrepareMake();
    void MakePos();
    // --> OD 2005-09-28 #b6329202#
    // method formats next frame of table frame to assure keep attribute.
    // in case of nested tables method <SwFrm::MakeAll()> is called to
    // avoid format of superior table frame.
    friend SwFrm* lcl_FormatNextCntntForKeep( SwTabFrm* pTabFrm );
    // <--
    virtual void MakeAll() = 0;
        //Adjustierung der Frames einer Seite
    SwTwips AdjustNeighbourhood( SwTwips nDiff, sal_Bool bTst = sal_False );


        //Aendern nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False ) = 0;
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False ) = 0;

    SwModify        *GetDep()       { return pRegisteredIn; }
    const SwModify  *GetDep() const { return pRegisteredIn; }

    SwFrm( SwModify* );

    void CheckDir( sal_uInt16 nDir, sal_Bool bVert, sal_Bool bOnlyBiDi, sal_Bool bBrowse );

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
        *every* assignment of the corresponding flag to <sal_False>.

        @author OD
    */
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid );

        //Schatten und Umrandung painten
    void PaintShadow( const SwRect&, SwRect&, const SwBorderAttrs& ) const;

public:
    TYPEINFO(); //Bereits in Basisklasse Client drin.

    sal_uInt16 GetType() const { return 0x1 << nType; }

    static SwCache &GetCache()                { return *pCache; }
    static SwCache *GetCachePtr()             { return pCache;  }
    static void     SetCache( SwCache *pNew ) { pCache = pNew;  }

        //Aendern die PrtArea-SSize und die FrmSize.
    SwTwips Shrink( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    SwTwips Grow  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

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
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, sal_Bool bFwd );
    SwLayoutFrm *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrm *GetNextFtnLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextCellLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetPrevLeaf   ( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevFtnLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevSctLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevCellLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    const SwLayoutFrm *GetLeaf ( MakePageType eMakePage, sal_Bool bFwd,
                                 const SwFrm *pAnch ) const;

    sal_Bool WrongPageDesc( SwPageFrm* pNew );

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
                    const sal_Bool bLowerBorder = sal_False ) const;
    void PaintBackground( const SwRect&, const SwPageFrm *pPage,
                          const SwBorderAttrs &,
                          const sal_Bool bLowerMode = sal_False,
                          const sal_Bool bLowerBorder = sal_False ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrm*,
                          const Color *pColor, const SvxBorderStyle = SOLID ) const;

    //Retouche, nicht im Bereich des uebergebenen Rect!
    void Retouche( const SwPageFrm *pPage, const SwRect &rRect ) const;

    sal_Bool GetBackgroundBrush( const SvxBrushItem*& rpBrush,
                             const Color*& rpColor,
                             SwRect &rOrigRect,
                             sal_Bool bLowerMode ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    inline sal_Bool IsCompletePaint() const { return bCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    inline sal_Bool IsRetouche() const { return bRetouche; }

    void SetInfFlags();                 //Setzen der InfoFlags
    inline void InvalidateInfFlags() { bInfInvalid = sal_True; }
    inline sal_Bool IsInDocBody() const;    //Benutzen die InfoFlags.
    inline sal_Bool IsInFtn() const;        //ggf. werden die Flags ermittelt.
    inline sal_Bool IsInTab() const;
    inline sal_Bool IsInFly() const;
    inline sal_Bool IsInSct() const;

    // If frame is inside a split table row, this function returns
    // the corresponding row frame in the follow table.
    const SwRowFrm* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
    const SwRowFrm* IsInFollowFlowRow() const;

    bool IsInBalancedSection() const;

    inline sal_Bool IsReverse() const { return bReverse; }
    inline void SetReverse( sal_Bool bNew ){ bReverse = bNew ? 1 : 0; }
    inline sal_Bool IsVertical() const;
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline sal_Bool IsVertLR() const;
    //End of SCMS
    inline sal_Bool GetVerticalFlag() const;
    inline void SetVertical( sal_Bool bNew ){ bVertical = bNew ? 1 : 0; }
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline void SetbVertLR( sal_Bool bNew ) { bVertLR = bNew ? 1 : 0; }
    //End of SCMS
    inline void SetDerivedVert( sal_Bool bNew ){ bDerivedVert = bNew ? 1 : 0; }
    inline void SetInvalidVert( sal_Bool bNew) { bInvalidVert = bNew ? 1 : 0; }
    inline sal_Bool IsRightToLeft() const;
    inline sal_Bool GetRightToLeftFlag() const;
    inline void SetRightToLeft( sal_Bool bNew ){ bRightToLeft = bNew ? 1 : 0; }
    inline void SetDerivedR2L( sal_Bool bNew ) { bDerivedR2L  = bNew ? 1 : 0; }
    inline void SetInvalidR2L( sal_Bool bNew ) { bInvalidR2L  = bNew ? 1 : 0; }

    void CheckDirChange();
    // returns upper left frame position for LTR and
    // upper right frame position for Asian / RTL frames
    Point   GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        OD 08.08.2003 #110978#
        method replaced 'old' method <sal_Bool IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrm == 0L), the movability in the actual
        environment (<this->GetUpper()) is checked.

        @author OD

        @param _pLayoutFrm
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0L ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
//    sal_Bool IsMoveable() const;
    bool IsMoveable( const SwLayoutFrm* _pLayoutFrm = 0L ) const;

    //Ist es fuer den (Txt)Frm in der aktuellen Umgebung erlaubt eine
    //Fussnote einzufuegen (nicht z.B. in wiederholten TabellenHeadlines).
    sal_Bool IsFtnAllowed() const;

    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );
    virtual void  Format( const SwBorderAttrs *pAttrs = 0 );

    virtual void  CheckDirection( sal_Bool bVert );

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;

    inline sal_Bool HasFixSize() const { return bFixSize; }
    inline void SetFixSize( sal_Bool bNew ) { bFixSize = bNew; }

    //Kann 0 liefern, pruefen auch ob die Shell zum richtigen Dokument
    //gehoert. Impl in frmsh.hxx
    ViewShell *GetShell() const;

    //Prueft alle Seiten ab der Uebergebenen und korrigiert ggf.
    static void CheckPageDescs( SwPageFrm *pStart, sal_Bool bNotifyFields = sal_True );

        //Koennen 0 liefern, einmal const einmal nicht
    SwFrm               *GetNext()  { return pNext; }
    SwFrm               *GetPrev()  { return pPrev; }
    SwLayoutFrm         *GetUpper() { return pUpper; }
    SwRootFrm           *FindRootFrm();
    SwPageFrm           *FindPageFrm();
    SwFrm               *FindColFrm();
    SwRowFrm            *FindRowFrm();
    SwFtnBossFrm        *FindFtnBossFrm( sal_Bool bFootnotes = sal_False );
    SwTabFrm            *ImplFindTabFrm();
    SwFtnFrm            *ImplFindFtnFrm();
    SwFlyFrm            *ImplFindFlyFrm();
    SwSectionFrm        *ImplFindSctFrm();
    SwFrm               *FindFooterOrHeader();
    SwFrm               *GetLower();
    const SwFrm         *GetNext()  const { return pNext; }
    const SwFrm         *GetPrev()  const { return pPrev; }
    const SwLayoutFrm   *GetUpper() const { return pUpper; }
    inline SwTabFrm     *FindTabFrm();
    inline SwFtnFrm     *FindFtnFrm();
    inline SwFlyFrm     *FindFlyFrm();
    inline SwSectionFrm *FindSctFrm();
    inline SwFrm        *FindNext();
    // --> OD 2005-12-01 #i27138# - add parameter <_bInSameFtn>
    inline SwCntntFrm* FindNextCnt( const bool _bInSameFtn = false );
    // <--
    inline SwFrm        *FindPrev();
    inline const SwPageFrm *FindPageFrm() const;
    inline const SwRootFrm *FindRootFrm() const;
    inline const SwFtnBossFrm *FindFtnBossFrm( sal_Bool bFtn = sal_False ) const;
    inline const SwFrm     *FindColFrm() const;
    inline const SwFrm     *FindFooterOrHeader() const;
    inline const SwTabFrm  *FindTabFrm() const;
    inline const SwFtnFrm  *FindFtnFrm() const;
    inline const SwFlyFrm  *FindFlyFrm() const;
    inline const SwSectionFrm *FindSctFrm() const;
    inline const SwFrm     *FindNext() const;
    // --> OD 2005-12-01 #i27138# - add parameter <_bInSameFtn>
    inline const SwCntntFrm* FindNextCnt( const bool _bInSameFtn = false ) const;
    // <--
    inline const SwFrm     *FindPrev() const;
           const SwFrm     *GetLower()  const;

    /** inline wrapper method for <_FindPrevCnt(..)>

        OD 2005-11-30 #i27138#

        @author OD
    */
    inline SwCntntFrm* FindPrevCnt( const bool _bInSameFtn = false )
    {
        if ( GetPrev() && GetPrev()->IsCntntFrm() )
            return (SwCntntFrm*)(GetPrev());
        else
            return _FindPrevCnt( _bInSameFtn );
    }

    /** inline const wrapper method for <_FindPrevCnt(..)>

        OD 2005-11-30 #i27138#

        @author OD
    */
    inline const SwCntntFrm* FindPrevCnt( const bool _bInSameFtn = false ) const
    {
        if ( GetPrev() && GetPrev()->IsCntntFrm() )
            return (const SwCntntFrm*)(GetPrev());
        else
            return const_cast<SwFrm*>(this)->_FindPrevCnt( _bInSameFtn );
    }

    // --> OD 2007-09-04 #i79774#, #b6596954#
    SwFrm* _GetIndPrev() const;
    SwFrm* GetIndPrev() const
        { return ( pPrev || !IsInSct() ) ? pPrev : _GetIndPrev(); }
//    const SwFrm* GetIndPrev() const { return ((SwFrm*)this)->GetIndPrev(); }
    // <--
    SwFrm* GetIndNext()
        { return ( pNext || !IsInSct() ) ? pNext : _GetIndNext(); }
    const SwFrm* GetIndNext() const { return ((SwFrm*)this)->GetIndNext(); }

    sal_uInt16 GetPhyPageNum() const;   //Seitennummer ohne Offset
    sal_uInt16 GetVirtPageNum() const;  //Seitenummer mit Offset
    sal_Bool OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    sal_Bool WannaRightPage() const;


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
    const SwRect UnionFrm( sal_Bool bBorder = sal_False ) const;

    //Der Zugriff auf die Member wird hier ausnahmsweiste gestattet,
    //dies soll aber nicht dazu dienen die Werte wahllos zu veraendern;
    //es ist nur die einzige Moeglichkeit die Compilerprobleme zu umgehen
    //(gleiche Methode mal public mal protected).
    SwRect &Frm() { return aFrm; }
    SwRect &Prt() { return aPrt; }

    virtual Size ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) = 0;

    void ValidateLineNum() { bValidLineNum = sal_True; }

    sal_Bool GetValidPosFlag()    const { return bValidPos; }
    sal_Bool GetValidPrtAreaFlag()const { return bValidPrtArea; }
    sal_Bool GetValidSizeFlag()   const { return bValidSize; }
    sal_Bool GetValidLineNumFlag()const { return bValidLineNum; }
    sal_Bool IsValid() const { return bValidPos && bValidSize && bValidPrtArea; }

    //Invalideren nur den Frm
    // OD 2004-05-19 #i28701# - add call to method <_ActionOnInvalidation(..)>
    // for all invalidation methods.
    // OD 2004-05-19 #i28701# - use method <_InvalidationAllowed(..)> to
    // decide, if invalidation will to be performed or not.
    // --> OD 2004-10-08 #i26945# - no additional invalidation, if it's already
    // invalidate.
    void _InvalidateSize()
    {
        if ( bValidSize && _InvalidationAllowed( INVALID_SIZE ) )
        {
            bValidSize = sal_False;
            _ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void _InvalidatePrt()
    {
        if ( bValidPrtArea && _InvalidationAllowed( INVALID_PRTAREA ) )
        {
            bValidPrtArea = sal_False;
            _ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void _InvalidatePos()
    {
        if ( bValidPos && _InvalidationAllowed( INVALID_POS ) )
        {
            bValidPos = sal_False;
            _ActionOnInvalidation( INVALID_POS );
        }
    }
    void _InvalidateLineNum()
    {
        if ( bValidLineNum && _InvalidationAllowed( INVALID_LINENUM ) )
        {
            bValidLineNum = sal_False;
            _ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void _InvalidateAll()
    {
        if ( ( bValidSize || bValidPrtArea || bValidPos ) &&
             _InvalidationAllowed( INVALID_ALL ) )
        {
            bValidSize = bValidPrtArea = bValidPos = sal_False;
            _ActionOnInvalidation( INVALID_ALL );
        }
    }
    // <--
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

    inline void InvalidateNextPos( sal_Bool bNoFtn = sal_False );
    void ImplInvalidateNextPos( sal_Bool bNoFtn = sal_False );

    /** method to invalidate printing area of next frame

        OD 09.01.2004 #i11859#

        @author OD
    */
    void InvalidateNextPrtArea();

    void InvalidatePage( const SwPageFrm *pPage = 0 ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    virtual sal_Bool    GetCrsrOfst( SwPosition *, Point&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual sal_Bool    GetCharRect( SwRect &, const SwPosition&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;

    // der "kurze Dienstweg" zwischen den Frames und der Formatierung.
    // Wer den void* falsch Casted ist selbst schuld!
    // Auf jedenfall muss der void* auf 0 geprueft werden.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

    //sal_True wenn's die richtige Klasse ist, sal_False sonst
    inline sal_Bool IsLayoutFrm() const;
    inline sal_Bool IsRootFrm() const;
    inline sal_Bool IsPageFrm() const;
    inline sal_Bool IsColumnFrm() const;
    inline sal_Bool IsFtnBossFrm() const; // Fussnotenbosse sind PageFrms und ColumnFrms
    inline sal_Bool IsHeaderFrm() const;
    inline sal_Bool IsFooterFrm() const;
    inline sal_Bool IsFtnContFrm() const;
    inline sal_Bool IsFtnFrm() const;
    inline sal_Bool IsBodyFrm() const;
    inline sal_Bool IsColBodyFrm() const;   // in layfrm.hxx implementiert, BodyFrm unterhalb ColumnFrm
    inline sal_Bool IsPageBodyFrm() const;  // in layfrm.hxx implementiert, BodyFrm unterhalb PageFrm
    inline sal_Bool IsFlyFrm() const;
    inline sal_Bool IsSctFrm() const;
    inline sal_Bool IsTabFrm() const;
    inline sal_Bool IsRowFrm() const;
    inline sal_Bool IsCellFrm() const;
    inline sal_Bool IsCntntFrm() const;
    inline sal_Bool IsTxtFrm() const;
    inline sal_Bool IsNoTxtFrm() const;
    inline sal_Bool IsFlowFrm() const;      //Frms deren PrtArea von den Nachbarn
                                        //abhaengen und die halt im Inhaltsfluss
                                        //stehen.
    inline sal_Bool IsRetoucheFrm() const;  //Frms die Retouchefaehig sind bzw. die
                                        //u.U. hinter sich Retouchieren muessen.
    inline sal_Bool IsAccessibleFrm() const;

    void PrepareCrsr();                 //Die CrsrShell darf.

    //Ist der Frm (bzw. die Section in der er steht) geschuetzt?
    //Auch Fly in Fly in ... und Fussnoten
    sal_Bool IsProtected() const;

    sal_Bool IsColLocked()  const { return bColLocked; }

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
    sal_Bool SetMinLeft( long );
    sal_Bool SetMaxBottom( long );
    sal_Bool SetMaxRight( long );
    sal_Bool SetMinTop( long );
    void MakeBelowPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeUpperPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeLeftPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeRightPos( const SwFrm*, const SwFrm*, sal_Bool );
    inline sal_Bool IsNeighbourFrm() const
        { return GetType() & FRM_NEIGHBOUR ? sal_True : sal_False; }

    // --> OD 2006-05-10 #i65250#
    inline sal_uInt32 GetFrmId() const { return mnFrmId; }
    inline sal_uInt32 GetLastFrmId() const { return mnLastFrmId; }
    // <--

    // NEW TABELS
    // Some functions for covered/covering table cells. This way unnessessary
    // includes can be avoided
    bool IsLeaveUpperAllowed() const;
    bool IsCoveredCell() const;
    bool IsInCoveredCell() const;

    // FME 2007-08-30 #i81146# new loop control
    void ValidateThisAndAllLowers( const sal_uInt16 nStage );

public:
#if OSL_DEBUG_LEVEL > 1
    virtual void dumpAsXml(xmlTextWriterPtr writer);
    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer);
    void dumpChildrenAsXml(xmlTextWriterPtr writer);
#endif
    bool IsCollapse() const;
};

inline sal_Bool SwFrm::IsInDocBody() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfBody;
}
inline sal_Bool SwFrm::IsInFtn() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFtn;
}
inline sal_Bool SwFrm::IsInTab() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfTab;
}
inline sal_Bool SwFrm::IsInFly() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFly;
}
inline sal_Bool SwFrm::IsInSct() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfSct;
}
sal_Bool SwFrm::IsVertical() const
{
    if( bInvalidVert )
        ((SwFrm*)this)->SetDirFlags( sal_True );
    return bVertical != 0;
}
//Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
inline sal_Bool SwFrm::IsVertLR() const
{
    return bVertLR != 0;
}
//End of SCMS
sal_Bool SwFrm::GetVerticalFlag() const
{
    return bVertical != 0;
}
inline sal_Bool SwFrm::IsRightToLeft() const
{
    if( bInvalidR2L )
        ((SwFrm*)this)->SetDirFlags( sal_False );
    return bRightToLeft != 0;
}
sal_Bool SwFrm::GetRightToLeftFlag() const
{
    return bRightToLeft != 0;
}

inline void SwFrm::SetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = sal_True;
}
inline void SwFrm::ResetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = sal_False;
}

inline void SwFrm::SetRetouche() const
{
    ((SwFrm*)this)->bRetouche = sal_True;
}
inline void SwFrm::ResetRetouche() const
{
    ((SwFrm*)this)->bRetouche = sal_False;
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
        bValidPrtArea = bValidSize = bValidPos = sal_False;

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_ALL );
    }
}

inline void SwFrm::InvalidateNextPos( sal_Bool bNoFtn )
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
inline const SwFtnBossFrm *SwFrm::FindFtnBossFrm( sal_Bool bFtn ) const
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
// --> OD 2005-12-01 #i27138# - add parameter <_bInSameFtn>
inline SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn )
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return _FindNextCnt( _bInSameFtn );
}
// <--
// --> OD 2005-12-01 #i27138# - add parameter <_bInSameFtn>
inline const SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn ) const
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return ((SwFrm*)this)->_FindNextCnt( _bInSameFtn );
}
// <--
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


inline sal_Bool SwFrm::IsLayoutFrm() const
{
    return GetType() & FRM_LAYOUT ? sal_True : sal_False;
}
inline sal_Bool SwFrm::IsRootFrm() const
{
    return nType == FRMC_ROOT;
}
inline sal_Bool SwFrm::IsPageFrm() const
{
    return nType == FRMC_PAGE;
}
inline sal_Bool SwFrm::IsColumnFrm() const
{
    return nType == FRMC_COLUMN;
}
inline sal_Bool SwFrm::IsFtnBossFrm() const
{
    return GetType() & FRM_FTNBOSS ? sal_True : sal_False;
}
inline sal_Bool SwFrm::IsHeaderFrm() const
{
    return nType == FRMC_HEADER;
}
inline sal_Bool SwFrm::IsFooterFrm() const
{
    return nType == FRMC_FOOTER;
}
inline sal_Bool SwFrm::IsFtnContFrm() const
{
    return nType == FRMC_FTNCONT;
}
inline sal_Bool SwFrm::IsFtnFrm() const
{
    return nType == FRMC_FTN;
}
inline sal_Bool SwFrm::IsBodyFrm() const
{
    return nType == FRMC_BODY;
}
inline sal_Bool SwFrm::IsFlyFrm() const
{
    return nType == FRMC_FLY;
}
inline sal_Bool SwFrm::IsSctFrm() const
{
    return nType == FRMC_SECTION;
}
inline sal_Bool SwFrm::IsTabFrm() const
{
    return nType == FRMC_TAB;
}
inline sal_Bool SwFrm::IsRowFrm() const
{
    return nType == FRMC_ROW;
}
inline sal_Bool SwFrm::IsCellFrm() const
{
    return nType == FRMC_CELL;
}
inline sal_Bool SwFrm::IsCntntFrm() const
{
    return GetType() & FRM_CNTNT ? sal_True : sal_False;
}
inline sal_Bool SwFrm::IsTxtFrm() const
{
    return nType == FRMC_TXT;
}
inline sal_Bool SwFrm::IsNoTxtFrm() const
{
    return nType == FRMC_NOTXT;
}
inline sal_Bool SwFrm::IsFlowFrm() const
{
    return GetType() & 0xCA00 ? sal_True : sal_False;   //TabFrm, CntntFrm, SectionFrm
}
inline sal_Bool SwFrm::IsRetoucheFrm() const
{
    return GetType() & 0xCA40 ? sal_True : sal_False;   //TabFrm, CntntFrm, SectionFrm, Ftnfrm
}
inline sal_Bool SwFrm::IsAccessibleFrm() const
{
    return GetType() & FRM_ACCESSIBLE ? sal_True : sal_False;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
