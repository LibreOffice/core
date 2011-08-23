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
#ifndef _PAGEFRM_HXX
#define _PAGEFRM_HXX



#include <bf_svtools/svarray.hxx>

#include "ftnboss.hxx"

#include <tools/mempool.hxx>
namespace binfilter {

class SvPtrarr; 
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


SV_DECL_PTRARR_SORT(SwSortDrawObjs,SdrObjectPtr,1,2)//STRIP008 ;

enum SwPageChg
{
    CHG_NEWPAGE,
    CHG_CUTPAGE,
    CHG_CHGPAGE
};

void SetLastPage( SwPageFrm* );

class SwPageFrm: public SwFtnBossFrm
{
    friend class SwFrm;

    //Array fuer die Draw-Objekte auf der Seite.
    //Das Sortierte Array ist nach den Pointeraddressen sortiert.
    //Die Objs in dem Array haben ihren Anker nicht notwendigerweise auf
    //der Seite.
    SwSortDrawObjs *pSortedObjs;

    SwPageDesc *pDesc;		//PageDesc der die Seite beschreibt.

    USHORT	nPhyPageNum;		//Physikalische Seitennummer.

    BOOL bInvalidCntnt		:1;
    BOOL bInvalidLayout		:1;
    BOOL bInvalidFlyCntnt	:1;
    BOOL bInvalidFlyLayout	:1;
    BOOL bInvalidFlyInCnt	:1;
    BOOL bFtnPage			:1;	//Diese Seite ist fuer Dokumentende-Fussnoten.
    BOOL bEmptyPage			:1; //Dies ist eine explizite Leerseite
    BOOL bInvalidSpelling	:1; //Das Online-Spelling ist gefordert
    BOOL bEndNotePage		:1; //'Fussnotenseite' fuer Endnoten
    BOOL bInvalidAutoCmplWrds :1; //Auto-Complete Wordliste aktualisieren
#ifdef VERTICAL_LAYOUT
    BOOL bHasGrid           :1; // Grid for Asian layout
#endif

    // OD 12.02.2003 #i9719#, #105645#
    static const sal_Int8 mnBorderPxWidth;
    static const sal_Int8 mnShadowPxWidth;

    //Anpassung der RootSize und Benachrichtigungen beim Einsetzen,
    //Entfernen und Groessenaenderungen der Seite.
    void AdjustRootSize( const SwPageChg eChgType, const SwRect *pOld );

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    // Anpassen der max. Fussnotenhoehen in den einzelnen Spalten

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
    void PrepareHeader();	//Kopf-/Fusszeilen anlegen/entfernen.
    void PrepareFooter();

    const SwSortDrawObjs  *GetSortedObjs() const  { return pSortedObjs; }
          SwSortDrawObjs  *GetSortedObjs()		  { return pSortedObjs; }

    void AppendDrawObj( SwDrawContact *pNew );
    void RemoveDrawObj( SwDrawContact *pToRemove );
        // OD 20.05.2003 #108784# - <AppendDrawObj>/<RemoveDrawObj> for virtual drawing objects
    void AppendVirtDrawObj( SwDrawContact* _pDrawContact,
                            SwDrawVirtObj* _pDrawVirtObj );
    void RemoveVirtDrawObj( SwDrawContact* _pDrawContact,
                            SwDrawVirtObj* _pDrawVirtObj );

    void AppendFly( SwFlyFrm *pNew );
    void RemoveFly( SwFlyFrm *pToRemove );
    void MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest );//optimiertes Remove/Append

    void  SetPageDesc( SwPageDesc *, SwFrmFmt * );
          SwPageDesc *GetPageDesc() { return pDesc; }
    const SwPageDesc *GetPageDesc() const { return pDesc; }
          SwPageDesc *FindPageDesc();

    SwCntntFrm  *FindLastBodyCntnt();
    inline 		 SwCntntFrm	 *FindFirstBodyCntnt();
    inline const SwCntntFrm  *FindFirstBodyCntnt() const;
    inline const SwCntntFrm  *FindLastBodyCntnt() const;

    //Spezialisiertes GetCntntPos() fuer Felder in Rahmen.
    void GetCntntPosition( const Point &rPt, SwPosition &rPos ) const;

    BOOL IsEmptyPage() const { return bEmptyPage; } //explizite Leerseite.

    void	UpdateFtnNum();

    //Immer nach dem Paste rufen. Erzeugt die Seitengeb. Rahmen und Formatiert
    //generischen Inhalt.
    void PreparePage( BOOL bFtn );

    //Schickt an alle ContentFrames ein Prepare wg. geaenderter Registervorlage
    void PrepareRegisterChg();

    //Haengt einen Fly an den geeigneten LayoutFrm unterhalb der Seite,
    //fuer SwFEShell und Modify der Flys.
    SwFrm *PlaceFly( SwFlyFrm *, SwFrmFmt *, const SwFmtAnchor * );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
        // erfrage vom Client Informationen

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#ifdef VERTICAL_LAYOUT
    virtual void  CheckDirection( BOOL bVert );
    void CheckGrid( BOOL bInvalidate );
    BOOL HasGrid() const { return bHasGrid; }
#endif

    //Zeilennummern usw malen

    //Hilfslinien malen.

    //Fussnotenschnittstelle
    BOOL IsFtnPage() const									{ return bFtnPage; }
    BOOL IsEndNotePage() const								{ return bEndNotePage; }
    void SetFtnPage( BOOL b )								{ bFtnPage = b; }
    void SetEndNotePage( BOOL b )							{ bEndNotePage = b; }

    inline	USHORT GetPhyPageNum() const	{ return nPhyPageNum;}
    inline	void SetPhyPageNum( USHORT nNum )	{ nPhyPageNum = nNum;}
    inline	void DecrPhyPageNum()				{ --nPhyPageNum;	 }
    inline	void IncrPhyPageNum()				{ ++nPhyPageNum;	 }

    //Validieren, invalidieren und abfragen des Status der Seite.
    //Layout/Cntnt und jeweils Fly/nicht Fly werden getrennt betrachtet.
    inline void InvalidateFlyLayout() const;
    inline void InvalidateFlyCntnt() const;
    inline void InvalidateFlyInCnt() const;
    inline void	InvalidateLayout() const;
    inline void	InvalidateCntnt() const;
    inline void	InvalidateSpelling() const;
    inline void	InvalidateAutoCompleteWords() const;
    inline void ValidateFlyLayout() const;
    inline void ValidateFlyCntnt() const;
    inline void ValidateFlyInCnt() const;
    inline void	ValidateLayout() const;
    inline void	ValidateCntnt()	const;
    inline void	ValidateSpelling()	const;
    inline void	ValidateAutoCompleteWords()	const;
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

    inline sal_Int8 BorderPxWidth() const
    {
        return mnBorderPxWidth;
    }
    inline sal_Int8 ShadowPxWidth() const
    {
        return mnShadowPxWidth;
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
inline void	SwPageFrm::InvalidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = TRUE;
}
inline void	SwPageFrm::InvalidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = TRUE;
}
inline void SwPageFrm::InvalidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = TRUE;
}
inline void	SwPageFrm::InvalidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = TRUE;
}
inline void	SwPageFrm::InvalidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = TRUE;
}
inline void	SwPageFrm::InvalidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = TRUE;
}
inline void	SwPageFrm::InvalidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = FALSE;
}
inline void	SwPageFrm::ValidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = FALSE;
}
inline void	SwPageFrm::ValidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = FALSE;
}
inline void SwPageFrm::ValidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = FALSE;
}
inline void	SwPageFrm::ValidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = FALSE;
}
inline void	SwPageFrm::ValidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = FALSE;
}
inline void	SwPageFrm::ValidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = FALSE;
}
inline void	SwPageFrm::ValidateAutoCompleteWords() const
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


} //namespace binfilter
#endif	//_PAGEFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
