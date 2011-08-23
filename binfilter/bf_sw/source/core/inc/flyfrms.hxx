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
#ifndef _FLYFRMS_HXX
#define _FLYFRMS_HXX

#include "flyfrm.hxx"
namespace binfilter {

//Basisklasse fuer diejenigen Flys, die sich relativ frei Bewegen koennen -
//also die nicht _im_ Inhalt gebundenen Flys.
class SwFlyFreeFrm : public SwFlyFrm
{
    SwPageFrm *pPage;	//Bei dieser Seite ist der Fly angemeldet.

    void CheckClip( const SwFmtFrmSize &rSz );	//'Emergency' Clipping.

protected:
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);


    SwFlyFreeFrm( SwFlyFrmFmt*, SwFrm *pAnchor );
public:

    virtual ~SwFlyFreeFrm();

    virtual void MakeAll();

          SwPageFrm *GetPage()		 { return pPage; }
    const SwPageFrm *GetPage() const { return pPage; }
    void  SetPage( SwPageFrm *pNew ) { pPage = pNew; }
};


//Die Fly's, die an einem Layoutfrm haengen und nicht inhaltsgebunden sind
class SwFlyLayFrm : public SwFlyFreeFrm
{
public:
    SwFlyLayFrm( SwFlyFrmFmt*, SwFrm *pAnchor );
    SwFlyLayFrm( SwFlyLayFrm& );

    virtual	void Modify( SfxPoolItem*, SfxPoolItem* );

};

//Die Flys, die an einem Cntnt haengen nicht aber im Inhalt
class SwFlyAtCntFrm : public SwFlyFreeFrm
{
    SwRect aLastCharRect; // Fuer autopositionierte Frames ( LAYER_IMPL )
protected:
    //Stellt sicher, das der Fly an der richtigen Seite haengt.
    void AssertPage();

    virtual void MakeAll();
    virtual void MakeFlyPos();
public:
    SwFlyAtCntFrm( SwFlyFrmFmt*, SwFrm *pAnchor );

    virtual	void Modify( SfxPoolItem*, SfxPoolItem* );


    // Fuer autopositionierte Frames ( LAYER_IMPL ), ueberprueft, ob sich
    // die Ankerposition geaendert hat und invalidiert ggf.

    SwTwips GetLastCharX() const { return aLastCharRect.Left() - Frm().Left(); }

    SwTwips GetRelCharX( const SwFrm* pFrm ) const
        { return aLastCharRect.Left() - pFrm->Frm().Left(); }
    SwTwips GetRelCharY( const SwFrm* pFrm ) const
        { return aLastCharRect.Bottom() - pFrm->Frm().Top(); }

    void AddLastCharY( long nDiff ){ aLastCharRect.Pos().Y() += nDiff; }
};

//Die Flys, die an einem Zeichen in einem Cntnt haengen.
class SwFlyInCntFrm : public SwFlyFrm
{
    Point aRef;	 //Relativ zu diesem Point wird die AbsPos berechnet.
    long  nLine; //Zeilenhoehe, Ref.Y() - nLine == Zeilenanfang.

    BOOL bInvalidLayout :1;
    BOOL bInvalidCntnt	:1;

    virtual void MakeFlyPos();

protected:
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);
    virtual void MakeAll();

public:
    SwFlyInCntFrm( SwFlyFrmFmt*, SwFrm *pAnchor );

    virtual ~SwFlyInCntFrm();
    virtual void  Format(  const SwBorderAttrs *pAttrs = 0 );
    virtual	void  Modify( SfxPoolItem*, SfxPoolItem* );

    void SetRefPoint( const Point& rPoint, const Point &rRelAttr,
        const Point &rRelPos );
    const Point &GetRefPoint() const { return aRef; }
    const Point &GetRelPos() const;
          long   GetLineHeight() const { return nLine; }

    inline void InvalidateLayout() const;
    inline void InvalidateCntnt() const;
    inline void ValidateLayout() const;
    inline void ValidateCntnt() const;
    BOOL IsInvalid() const { return (bInvalidLayout || bInvalidCntnt); }
    BOOL IsInvalidLayout() const { return bInvalidLayout; }
    BOOL IsInvalidCntnt() const { return bInvalidCntnt; }


    //BP 26.11.93: vgl. tabfrm.hxx, gilt bestimmt aber fuer andere auch...
    //Zum Anmelden der Flys nachdem ein FlyCnt erzeugt _und_ eingefuegt wurde.
    //Muss vom Erzeuger gerufen werden, denn erst nach dem Konstruieren wird
    //Das Teil gepastet; mithin ist auch erst dann die Seite zum Anmelden der
    //Flys erreichbar.
    void RegistFlys();

    //siehe layact.cxx
    void AddRefOfst( long nOfst ) { aRef.Y() += nOfst; }
};

inline void SwFlyInCntFrm::InvalidateLayout() const
{
    ((SwFlyInCntFrm*)this)->bInvalidLayout = TRUE;
}
inline void SwFlyInCntFrm::InvalidateCntnt() const
{
    ((SwFlyInCntFrm*)this)->bInvalidCntnt = TRUE;
}
inline void SwFlyInCntFrm::ValidateLayout() const
{
    ((SwFlyInCntFrm*)this)->bInvalidLayout = FALSE;
}
inline void SwFlyInCntFrm::ValidateCntnt() const
{
    ((SwFlyInCntFrm*)this)->bInvalidCntnt = FALSE;
}

} //namespace binfilter
#endif
