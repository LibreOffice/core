/*************************************************************************
 *
 *  $RCSfile: pagefrm.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:06:36 $
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
#ifndef _PAGEFRM_HXX
#define _PAGEFRM_HXX



#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "ftnboss.hxx"

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

class SwFlyFrm;
class SwFrmFmt;
class SwPageDesc;
class SwCntntFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SwFmtAnchor;
class SdrObject;
class SwDrawContact;
class SwAttrSetChg;
class SvPtrarr;

// OD 2004-05-07 #i28701# - replaced by class <SwSortedObjs>
//SV_DECL_PTRARR_SORT(SwSortDrawObjs,SdrObjectPtr,1,2);
class SwSortedObjs;
// --> OD 2004-07-02 #i28701#
class SwAnchoredObject;
// <--

enum SwPageChg
{
    CHG_NEWPAGE,
    CHG_CUTPAGE,
    CHG_CHGPAGE
};

class SwPageFrm: public SwFtnBossFrm
{
    friend class SwFrm;

    // OD 2004-05-07 #i28701# - use <SwSortedObjs>
    SwSortedObjs *pSortedObjs;

    SwPageDesc *pDesc;      //PageDesc der die Seite beschreibt.

    USHORT  nPhyPageNum;        //Physikalische Seitennummer.

    BOOL bInvalidCntnt      :1;
    BOOL bInvalidLayout     :1;
    BOOL bInvalidFlyCntnt   :1;
    BOOL bInvalidFlyLayout  :1;
    BOOL bInvalidFlyInCnt   :1;
    BOOL bFtnPage           :1; //Diese Seite ist fuer Dokumentende-Fussnoten.
    BOOL bEmptyPage         :1; //Dies ist eine explizite Leerseite
    BOOL bInvalidSpelling   :1; //Das Online-Spelling ist gefordert
    BOOL bEndNotePage       :1; //'Fussnotenseite' fuer Endnoten
    BOOL bInvalidAutoCmplWrds :1; //Auto-Complete Wordliste aktualisieren
    BOOL bHasGrid           :1; // Grid for Asian layout

    // OD 2004-05-17 #i28701# - boolean, indicating that layout of page frame
    // is in progress.
    bool mbLayoutInProgress;

    // OD 12.02.2003 #i9719#, #105645#
    static const sal_Int8 mnBorderPxWidth;
    static const sal_Int8 mnShadowPxWidth;

    //Anpassung der RootSize und Benachrichtigungen beim Einsetzen,
    //Entfernen und Groessenaenderungen der Seite.
    void AdjustRootSize( const SwPageChg eChgType, const SwRect *pOld );

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    // Anpassen der max. Fussnotenhoehen in den einzelnen Spalten
    void SetColMaxFtnHeight();

    /** determine rectangle for page border

        OD 12.02.2003 for #i9719# and #105645#

        @author OD

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orBorderRect
        output parameter - instance reference of the border rectangle for
        the given page rectangle
    */
    void GetBorderRect( const SwRect& _rPageRect,
                        ViewShell*    _pViewShell,
                        SwRect& _orBorderRect ) const;

    /** determine rectangle for right page shadow

        OD 12.02.2003 for #i9719# and #105645#

        @author OD

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orRightShadowRect
        output parameter - instance reference of the right shadow rectangle for
        the given page rectangle
    */
    void GetRightShadowRect( const SwRect& _rPageRect,
                             ViewShell*    _pViewShell,
                             SwRect&       _orRightShadowRect ) const;

    /** determine rectangle for bottom page shadow

        OD 12.02.2003 for #i9719# and #105645#

        @author OD

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orBottomShadowRect
        output parameter - instance reference of the bottom shadow rectangle for
        the given page rectangle
    */
    void GetBottomShadowRect( const SwRect& _rPageRect,
                              ViewShell*    _pViewShell,
                              SwRect&       _orBottomShadowRect ) const;

protected:
    virtual void MakeAll();

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwPageFrm)

    SwPageFrm( SwFrmFmt*, SwPageDesc* );
    ~SwPageFrm();

    //public, damit die ViewShell beim Umschalten vom BrowseMode darauf
    //zugreifen kann.
    void PrepareHeader();   //Kopf-/Fusszeilen anlegen/entfernen.
    void PrepareFooter();

    const SwSortedObjs  *GetSortedObjs() const  { return pSortedObjs; }
          SwSortedObjs  *GetSortedObjs()          { return pSortedObjs; }

    // --> OD 2004-07-02 #i28701# - new methods to append/remove drawing objects
    void AppendDrawObjToPage( SwAnchoredObject& _rNewObj );
    void RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj );
    // <--

    void AppendFlyToPage( SwFlyFrm *pNew );
    void RemoveFlyFromPage( SwFlyFrm *pToRemove );
    void MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest );//optimiertes Remove/Append

    void  SetPageDesc( SwPageDesc *, SwFrmFmt * );
          SwPageDesc *GetPageDesc() { return pDesc; }
    const SwPageDesc *GetPageDesc() const { return pDesc; }
          SwPageDesc *FindPageDesc();

                 SwCntntFrm  *FindLastBodyCntnt();
    inline       SwCntntFrm  *FindFirstBodyCntnt();
    inline const SwCntntFrm  *FindFirstBodyCntnt() const;
    inline const SwCntntFrm  *FindLastBodyCntnt() const;

    //Spezialisiertes GetCntntPos() fuer Felder in Rahmen.
    void GetCntntPosition( const Point &rPt, SwPosition &rPos ) const;

    BOOL IsEmptyPage() const { return bEmptyPage; } //explizite Leerseite.

    void    UpdateFtnNum();

    //Immer nach dem Paste rufen. Erzeugt die Seitengeb. Rahmen und Formatiert
    //generischen Inhalt.
    void PreparePage( BOOL bFtn );

    //Schickt an alle ContentFrames ein Prepare wg. geaenderter Registervorlage
    void PrepareRegisterChg();

    //Haengt einen Fly an den geeigneten LayoutFrm unterhalb der Seite,
    //fuer SwFEShell und Modify der Flys.
    SwFrm *PlaceFly( SwFlyFrm *, SwFrmFmt *, const SwFmtAnchor * );

    virtual BOOL GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#ifdef VERTICAL_LAYOUT
    virtual void  CheckDirection( BOOL bVert );
    void CheckGrid( BOOL bInvalidate );
    void PaintGrid( OutputDevice* pOut, SwRect &rRect ) const;
    BOOL HasGrid() const { return bHasGrid; }
#endif

    //Zeilennummern usw malen
    void RefreshExtraData( const SwRect & ) const;

    //Hilfslinien malen.
    void RefreshSubsidiary( const SwRect& ) const;

    //Fussnotenschnittstelle
    BOOL IsFtnPage() const                                  { return bFtnPage; }
    BOOL IsEndNotePage() const                              { return bEndNotePage; }
    void SetFtnPage( BOOL b )                               { bFtnPage = b; }
    void SetEndNotePage( BOOL b )                           { bEndNotePage = b; }

    inline  const USHORT GetPhyPageNum() const  { return nPhyPageNum;}
    inline  void SetPhyPageNum( USHORT nNum )   { nPhyPageNum = nNum;}
    inline  void DecrPhyPageNum()               { --nPhyPageNum;     }
    inline  void IncrPhyPageNum()               { ++nPhyPageNum;     }

    //Validieren, invalidieren und abfragen des Status der Seite.
    //Layout/Cntnt und jeweils Fly/nicht Fly werden getrennt betrachtet.
    inline void InvalidateFlyLayout() const;
    inline void InvalidateFlyCntnt() const;
    inline void InvalidateFlyInCnt() const;
    inline void InvalidateLayout() const;
    inline void InvalidateCntnt() const;
    inline void InvalidateSpelling() const;
    inline void InvalidateAutoCompleteWords() const;
    inline void ValidateFlyLayout() const;
    inline void ValidateFlyCntnt() const;
    inline void ValidateFlyInCnt() const;
    inline void ValidateLayout() const;
    inline void ValidateCntnt() const;
    inline void ValidateSpelling()  const;
    inline void ValidateAutoCompleteWords() const;
    inline BOOL IsInvalid() const;
    inline BOOL IsInvalidFly() const;
    BOOL IsInvalidFlyLayout() const { return bInvalidFlyLayout; }
    BOOL IsInvalidFlyCntnt() const { return bInvalidFlyCntnt; }
    BOOL IsInvalidFlyInCnt() const { return bInvalidFlyInCnt; }
    BOOL IsInvalidLayout() const { return bInvalidLayout; }
    BOOL IsInvalidCntnt() const { return (bInvalidCntnt || bInvalidFlyInCnt); }
    BOOL IsInvalidSpelling() const { return bInvalidSpelling; }
    BOOL IsInvalidAutoCompleteWords() const { return bInvalidAutoCmplWrds; }

    /** SwPageFrm::GetDrawBackgrdColor - for #102450#

        29.08.2002:
        determine the color, that is respectively will be drawn as background
        for the page frame.

        @author OD

        @return reference to an instance of class Color
    */
    const Color& GetDrawBackgrdColor() const;

    /** paint margin area of a page

        OD 20.11.2002 for #104598#:
        implement paint of margin area; margin area will be painted for a
        view shell with a window and if the document is not in online layout.

        @author OD

        @param _rOutputRect
        input parameter - constant instance reference of the rectangle, for
        which an output has to be generated.

        @param _pViewShell
        input parameter - instance of the view shell, on which the output
        has to be generated.
    */
    void PaintMarginArea( const SwRect& _rOutputRect,
                          ViewShell* _pViewShell ) const;

    /** paint page border and shadow

        OD 12.02.2003 for #i9719# and #105645#
        implement paint of page border and shadow

        @author OD

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, on which the output
        has to be generated.
    */
    void PaintBorderAndShadow( const SwRect& _rPageRect,
                               ViewShell*    _pViewShell ) const;

    /** get bound rectangle of border and shadow for repaints

        OD 12.02.2003 for #i9719# and #105645#

        author OD

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orBorderAndShadowBoundRect
        output parameter - instance reference of the bounded border and shadow
        rectangle for the given page rectangle
    */
    void GetBorderAndShadowBoundRect( const SwRect& _rPageRect,
                                      ViewShell*    _pViewShell,
                                      SwRect& _orBorderAndShadowBoundRect ) const;

    // OD 12.02.2003 #i9719#, #105645#
    inline sal_Int8 BorderPxWidth() const
    {
        return mnBorderPxWidth;
    }
    inline sal_Int8 ShadowPxWidth() const
    {
        return mnShadowPxWidth;
    }

    // OD 22.09.2003 #110978#
    const SwRect PrtWithoutHeaderAndFooter() const;

    // OD 2004-05-17 #i28701#
    inline bool IsLayoutInProgress() const
    {
        return mbLayoutInProgress;
    }
    inline void SetLayoutInProgress( const bool _bLayoutInProgress )
    {
        mbLayoutInProgress = _bLayoutInProgress;
    }

};

inline SwCntntFrm *SwPageFrm::FindFirstBodyCntnt()
{
    SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsCntnt() : 0;
}
inline const SwCntntFrm *SwPageFrm::FindFirstBodyCntnt() const
{
    const SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsCntnt() : 0;
}
inline const SwCntntFrm *SwPageFrm::FindLastBodyCntnt() const
{
    return ((SwPageFrm*)this)->FindLastBodyCntnt();
}
inline void SwPageFrm::InvalidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = TRUE;
}
inline void SwPageFrm::InvalidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = TRUE;
}
inline void SwPageFrm::InvalidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = TRUE;
}
inline void SwPageFrm::InvalidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = TRUE;
}
inline void SwPageFrm::InvalidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = TRUE;
}
inline void SwPageFrm::InvalidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = TRUE;
}
inline void SwPageFrm::InvalidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = FALSE;
}
inline void SwPageFrm::ValidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = FALSE;
}
inline void SwPageFrm::ValidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = FALSE;
}
inline void SwPageFrm::ValidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = FALSE;
}
inline void SwPageFrm::ValidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = FALSE;
}
inline void SwPageFrm::ValidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = FALSE;
}
inline void SwPageFrm::ValidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = FALSE;
}
inline void SwPageFrm::ValidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = FALSE;
}


inline BOOL SwPageFrm::IsInvalid() const
{
    return (bInvalidCntnt || bInvalidLayout || bInvalidFlyInCnt);
}
inline BOOL SwPageFrm::IsInvalidFly() const
{
    return bInvalidFlyLayout || bInvalidFlyCntnt;
}

#ifdef VERTICAL_LAYOUT
#define GETGRID( pPage ) const SwTextGridItem *pGrid = NULL; \
 {if( pPage && pPage->HasGrid() && GRID_NONE==(pGrid=(SwTextGridItem*)&pPage->\
     GetPageDesc()->GetMaster().GetAttr(RES_TEXTGRID))->GetGridType() ) \
    pGrid = NULL;}
#endif


#endif  //_PAGEFRM_HXX
