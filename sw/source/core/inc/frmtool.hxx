/*************************************************************************
 *
 *  $RCSfile: frmtool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:29:57 $
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
#ifndef _FRMTOOL_HXX
#define _FRMTOOL_HXX

#include "swtypes.hxx"
#include "layfrm.hxx"
#include "frmatr.hxx"
#include "swcache.hxx"

class SwPageFrm;
class SwFlyFrm;
class SwCntntFrm;
class SwFtnContFrm;
class SwDoc;
class SwAttrSet;
class SdrObject;
class BigInt;
class SvxBrushItem;
class SdrMarkList;
class SwNodeIndex;
class OutputDevice;
class SwPageDesc;
class SwCrsrShell;

#if defined(MSC)
#define MA_FASTCALL __fastcall
#else
#define MA_FASTCALL
#endif

#define WEIT_WECH       LONG_MAX - 20000        //Initale Position der Flys.
#define BROWSE_HEIGHT   56700L * 10L                //10 Meter

#define GRFNUM_NO 0
#define GRFNUM_YES 1
#define GRFNUM_REPLACE 2

//Painten des Hintergrunds. Mit Brush oder Graphic.
void MA_FASTCALL DrawGraphic( const SvxBrushItem *, OutputDevice *,
      const SwRect &rOrg, const SwRect &rOut, const BYTE nGrfNum = GRFNUM_NO );

//Fly besorgen, wenn keine List hineingereicht wird, wir die der aktuellen
//Shell benutzt.
//Implementierung in feshview.cxx
SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, ViewShell *pSh );

//Nicht gleich die math.lib anziehen.
ULONG MA_FASTCALL SqRt( BigInt nX );

//CntntNodes besorgen, CntntFrms erzeugen und in den LayFrm haengen.
void MA_FASTCALL _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc, ULONG nIndex,
                 BOOL bPages = FALSE, ULONG nEndIndex = 0,
                 SwFrm *pPrv = 0 );

//Erzeugen der Frames fuer einen bestimmten Bereich, verwendet _InsertCnt
void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
                            const SwNodeIndex &rEndIdx );

//Um z.B. fuer Tabelleheadlines das Erzeugen der Flys in _InsertCnt zu unterbinden.
extern FASTBOOL bDontCreateObjects;

//Fuer FlyCnts, siehe SwFlyAtCntFrm::MakeAll()
extern FASTBOOL bSetCompletePaintOnInvalidate;

//Rechteck fuer BorderPaint ausdehen/wieder schrumpfen.
void MA_FASTCALL SizeBorderRect( SwRect &rRect );
void MA_FASTCALL InvertSizeBorderRect( SwRect &rRect, ViewShell *pSh );

//Fuer Tabelleneinstellung per Tastatur.
long MA_FASTCALL CalcRowRstHeight( SwLayoutFrm *pRow );
long MA_FASTCALL CalcHeightWidthFlys( const SwFrm *pFrm );  //MA_FLY_HEIGHT

//Neue Seite einsetzen
SwPageFrm * MA_FASTCALL InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          BOOL bOdd, BOOL bInsertEmpty, BOOL bFtn,
                          SwFrm *pSibling );

//Flys bei der Seite anmelden.
void RegistFlys( SwPageFrm*, const SwLayoutFrm* );

//Benachrichtung des Fly Hintergrundes wenn Notwendig.
void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld );

void Notify_Background( SdrObject *pObj, SwPageFrm *pPage, const SwRect& rRect,
                        const PrepareHint eHint, const BOOL bInva );

const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos );

BOOL Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj );

const SwFrm *FindKontext( const SwFrm *pFrm, USHORT nAdditionalKontextTyp );

BOOL IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm );

void MA_ParkCrsr( SwPageDesc *pDesc, SwCrsrShell &rSh );

const SwFrm * MA_FASTCALL FindPage( const SwRect &rRect, const SwFrm *pPage );

// JP 07.05.98: wird von SwCntntNode::GetFrm und von SwFlyFrm::GetFrm
//              gerufen
SwFrm* GetFrmOfModify( SwModify&, USHORT nFrmType, const Point* = 0,
                        const SwPosition *pPos = 0,
                        const BOOL bCalcFrm = FALSE );

//Sollen ExtraDaten (Reline-Strich, Zeilennummern) gepaintet werden?
FASTBOOL IsExtraData( const SwDoc *pDoc );


//Die Notify-Klassen merken sich im CTor die aktuellen Groessen und fuehren
//im DTor ggf. die notwendigen Benachrichtigungen durch.

class SwFrmNotify
{
protected:
    SwFrm *pFrm;
    const SwRect aFrm;
    const SwRect aPrt;
    FASTBOOL     bHadFollow;
    FASTBOOL     bInvaKeep;
public:
    SwFrmNotify( SwFrm *pFrm );
    ~SwFrmNotify();

    const SwRect &Frm() const { return aFrm; }
    const SwRect &Prt() const { return aPrt; }
    void SetInvaKeep() { bInvaKeep = TRUE; }
};

class SwLayNotify : public SwFrmNotify
{
    SwTwips  nHeightOfst;
    SwTwips  nWidthOfst;
    FASTBOOL bLowersComplete;

    SwLayoutFrm *GetLay() { return (SwLayoutFrm*)pFrm; }
public:
    SwLayNotify( SwLayoutFrm *pLayFrm );
    ~SwLayNotify();

    void AddHeightOfst     ( SwTwips nAdd ) { nHeightOfst += nAdd; }
    void AddWidthOfst      ( SwTwips nAdd ) { nHeightOfst += nAdd; }
    void SubtractHeightOfst( SwTwips nSub ) { nWidthOfst -= nSub; }
    void SubtractWidthOfst ( SwTwips nSub ) { nWidthOfst -= nSub; }
    SwTwips GetHeightOfst() const { return nHeightOfst; }
    SwTwips GetWidthOfst()  const { return nWidthOfst; }
    void    ResetHeightOfst() { nHeightOfst = 0; }
    void    ResetWidthOfst()  { nWidthOfst = 0; }

    void SetLowersComplete( FASTBOOL b ) { bLowersComplete = b; }
    FASTBOOL IsLowersComplete()          { return bLowersComplete; }
};

class SwFlyNotify : public SwLayNotify
{
    SwPageFrm *pOldPage;
    const SwRect aFrmAndSpace;
    SwFlyFrm *GetFly() { return (SwFlyFrm*)pFrm; }
public:
    SwFlyNotify( SwFlyFrm *pFlyFrm );
    ~SwFlyNotify();

    SwPageFrm *GetOldPage() const { return pOldPage; }
};

class SwCntntNotify : public SwFrmNotify
{
    SwCntntFrm *GetCnt() { return (SwCntntFrm*)pFrm; }
public:
    SwCntntNotify( SwCntntFrm *pCntFrm );
    ~SwCntntNotify();
};

//SwBorderAttrs kapselt die Berechnung fuer die Randattribute inclusive
//Umrandung. Die Attribute und die errechneten Werte werden gecached.
//Neu: Die gesammte Klasse wird gecached.

//!!!Achtung: Wenn weitere Attribute gecached werden muss unbedingt die
//Methode Modify::Modify mitgepflegt werden!!!

class SwBorderAttrs : public SwCacheObj
{
    const SwAttrSet      &rAttrSet;
    const SvxULSpaceItem &rUL;
    const SvxLRSpaceItem &rLR;
    const SvxBoxItem     &rBox;
    const SvxShadowItem  &rShadow;
    const Size            aFrmSize;     //Die FrmSize
    long nRight;

    BOOL bBorderDist    :1;             //Ist's ein Frm der auch ohne Linie
                                        //einen Abstand haben kann?

    //Mit den Folgenden Bools werden die gecache'ten Werte fuer UNgueltig
    //erklaert - bis sie einmal berechnet wurden.
    BOOL bTopLine       :1;
    BOOL bBottomLine    :1;
    BOOL bLeftLine      :1;
    BOOL bRightLine     :1;
    BOOL bTop           :1;
    BOOL bBottom        :1;
    BOOL bRight         :1;
    BOOL bLine          :1;

    BOOL bIsLine        :1; //Umrandung an mind. einer Kante?

    BOOL bCacheGetLine        :1; //GetTopLine(), GetBottomLine() cachen?
    BOOL bCachedGetTopLine    :1; //GetTopLine() gecached?
    BOOL bCachedGetBottomLine :1; //GetTopLine() gecached?

    //Die gecache'ten Werte, undefiniert bis sie einmal berechnet wurden.
    USHORT nTopLine,
           nBottomLine,
           nLeftLine,
           nRightLine,
           nTop,
           nBottom,
           nGetTopLine,
           nGetBottomLine;

    //Nur die Lines + Shadow errechnen.
    void _CalcTopLine();
    void _CalcBottomLine();
    void _CalcLeftLine();
    void _CalcRightLine();

    //Lines + Shadow + Abstaende
    void _CalcTop();
    void _CalcBottom();
    void _CalcRight();

    void _IsLine();

    void _GetTopLine   ( const SwFrm *pFrm );
    void _GetBottomLine( const SwFrm *pFrm );

     //Rechte und linke Linie sowie LRSpace gleich?
    BOOL CmpLeftRight( const SwBorderAttrs &rAttrs, const SwFrm *pCaller,
                       const SwFrm *pCmp ) const;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwBorderAttrs)

    SwBorderAttrs( const SwModify *pOwner, const SwFrm *pConstructor );
    ~SwBorderAttrs();

    inline const SwAttrSet      &GetAttrSet() const { return rAttrSet;  }
    inline const SvxULSpaceItem &GetULSpace() const { return rUL;       }
    inline const SvxLRSpaceItem &GetLRSpace() const { return rLR;       }
    inline const SvxBoxItem     &GetBox()     const { return rBox;      }
    inline const SvxShadowItem  &GetShadow()  const { return rShadow;   }

    inline USHORT CalcTopLine() const;
    inline USHORT CalcBottomLine() const;
    inline USHORT CalcLeftLine() const;
    inline USHORT CalcRightLine() const;
    inline USHORT CalcTop() const;
    inline USHORT CalcBottom() const;
           long CalcLeft( const SwFrm *pCaller ) const;
    inline long CalcRight() const;

    inline BOOL IsLine() const;

    inline const Size &GetSize()     const { return aFrmSize; }

    inline BOOL IsBorderDist() const { return bBorderDist; }

    //Sollen obere bzw. untere Umrandung fuer den Frm ausgewertet werden?
    inline USHORT GetTopLine   ( const SwFrm *pFrm ) const;
    inline USHORT GetBottomLine( const SwFrm *pFrm ) const;
    inline void   SetGetCacheLine( BOOL bNew ) const;
};

class SwBorderAttrAccess : public SwCacheAccess
{
    const SwFrm *pConstructor;      //opt: Zur weitergabe an SwBorderAttrs
protected:
    virtual SwCacheObj *NewObj();

public:
    SwBorderAttrAccess( SwCache &rCache, const SwFrm *pOwner );

    SwBorderAttrs *Get();
};

//---------------------------------------------------------------------
//Iterator fuer die DrawObjecte einer Seite. Die Objecte werden Nach ihrer
//Z-Order iteriert.
//Das iterieren ist nicht eben billig, denn fuer alle Operationen muss jeweils
//ueber das gesamte SortArray iteriert werden.
class SwOrderIter
{
    const SwPageFrm *pPage;
    const SdrObject *pCurrent;
    const FASTBOOL bFlysOnly;
public:
    SwOrderIter( const SwPageFrm *pPage, FASTBOOL bFlysOnly = TRUE );

    void             Current( const SdrObject *pNew ) { pCurrent = pNew; }
    const SdrObject *Current()    const { return pCurrent; }
    const SdrObject *operator()() const { return pCurrent; }
    const SdrObject *Top();
    const SdrObject *Bottom();
    const SdrObject *Next();
    const SdrObject *Prev();
};


class StackHack
{
    static BYTE nCnt;
    static BOOL bLocked;
public:
    StackHack()
    {
        if ( ++StackHack::nCnt > 50 )
            StackHack::bLocked = TRUE;
    }
    ~StackHack()
    {
        if ( --StackHack::nCnt == 0 )
            StackHack::bLocked = FALSE;
    }

    static BOOL IsLocked()  { return StackHack::bLocked; }
    static BYTE Count()     { return StackHack::nCnt; }
};


//Sollen obere bzw. untere Umrandung fuer den Frm ausgewertet werden?
inline USHORT SwBorderAttrs::GetTopLine   ( const SwFrm *pFrm ) const
{
    if ( !bCachedGetTopLine )
        ((SwBorderAttrs*)this)->_GetTopLine( pFrm );
    return nGetTopLine;
}
inline USHORT SwBorderAttrs::GetBottomLine( const SwFrm *pFrm ) const
{
    if ( !bCachedGetBottomLine )
        ((SwBorderAttrs*)this)->_GetBottomLine( pFrm );
    return nGetBottomLine;
}
inline void SwBorderAttrs::SetGetCacheLine( BOOL bNew ) const
{
    ((SwBorderAttrs*)this)->bCacheGetLine = bNew;
    ((SwBorderAttrs*)this)->bCachedGetBottomLine =
    ((SwBorderAttrs*)this)->bCachedGetTopLine = FALSE;
}

inline USHORT SwBorderAttrs::CalcTopLine() const
{
    if ( bTopLine )
        ((SwBorderAttrs*)this)->_CalcTopLine();
    return nTopLine;
}
inline USHORT SwBorderAttrs::CalcBottomLine() const
{
    if ( bBottomLine )
        ((SwBorderAttrs*)this)->_CalcBottomLine();
    return nBottomLine;
}
inline USHORT SwBorderAttrs::CalcLeftLine() const
{
    if ( bLeftLine )
        ((SwBorderAttrs*)this)->_CalcLeftLine();
    return nLeftLine;
}
inline USHORT SwBorderAttrs::CalcRightLine() const
{
    if ( bRightLine )
        ((SwBorderAttrs*)this)->_CalcRightLine();
    return nRightLine;
}
inline USHORT SwBorderAttrs::CalcTop() const
{
    if ( bTop )
        ((SwBorderAttrs*)this)->_CalcTop();
    return nTop;
}
inline USHORT SwBorderAttrs::CalcBottom() const
{
    if ( bBottom )
        ((SwBorderAttrs*)this)->_CalcBottom();
    return nBottom;
}
inline long SwBorderAttrs::CalcRight() const
{
    if ( bRight )
        ((SwBorderAttrs*)this)->_CalcRight();
    return nRight;
}
inline BOOL SwBorderAttrs::IsLine() const
{
    if ( bLine )
        ((SwBorderAttrs*)this)->_IsLine();
    return bIsLine;
}

#endif  //_FRMTOOL_HXX
