/*************************************************************************
 *
 *  $RCSfile: flyfrms.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-03-08 13:58:18 $
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
#ifndef _FLYFRMS_HXX
#define _FLYFRMS_HXX

#include "flyfrm.hxx"

// OD 29.10.2003 #110978#
namespace objectpositioning
{
    class SwToCntntAnchoredObjectPosition;
}
// OD 11.11.2003 #i22341#
class SwFmtAnchor;

//Basisklasse fuer diejenigen Flys, die sich relativ frei Bewegen koennen -
//also die nicht _im_ Inhalt gebundenen Flys.
class SwFlyFreeFrm : public SwFlyFrm
{
    SwPageFrm *pPage;   //Bei dieser Seite ist der Fly angemeldet.

    void CheckClip( const SwFmtFrmSize &rSz );  //'Emergency' Clipping.

    /** determines, if direct environment of fly frame has 'auto' size

        OD 07.08.2003 #i17297#, #111066#, #111070#
        start with anchor frame and search for a header, footer, row or fly frame
        stopping at page frame.
        return <true>, if such a frame is found and it has 'auto' size.
        otherwise <false> is returned.

        @author OD

        @return boolean indicating, that direct environment has 'auto' size
    */
    bool HasEnvironmentAutoSize() const;

protected:
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);


    SwFlyFreeFrm( SwFlyFrmFmt*, SwFrm *pAnchor );
public:

    virtual ~SwFlyFreeFrm();

    virtual void MakeAll();

          SwPageFrm *GetPage()       { return pPage; }
    const SwPageFrm *GetPage() const { return pPage; }
    void  SetPage( SwPageFrm *pNew ) { pPage = pNew; }

};


//Die Fly's, die an einem Layoutfrm haengen und nicht inhaltsgebunden sind
class SwFlyLayFrm : public SwFlyFreeFrm
{
public:
    SwFlyLayFrm( SwFlyFrmFmt*, SwFrm *pAnchor );
    SwFlyLayFrm( SwFlyLayFrm& );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );

};

//Die Flys, die an einem Cntnt haengen nicht aber im Inhalt
class SwFlyAtCntFrm : public SwFlyFreeFrm
{
    // OD 29.10.2003 #110978# - access for calculation of position
    friend class objectpositioning::SwToCntntAnchoredObjectPosition;
    // OD 06.10.2003 #110978# - data of position calculation
    // layout frame vertical position is orient at - typically its the upper of
    // the anchor frame, but it could also by the upper of a follow or
    // a following layout frame in the text flow.
    const SwLayoutFrm* mpVertPosOrientFrm;

    // Last known anchor character retangle of to-character anchored Writer
    // fly frames.
    // Used to decide, if invalidation has to been performed, if anchor position
    // has changed, and used to position Writer fly frame.
    SwRect maLastCharRect;

    // OD 11.11.2003 #i22341# - for to-character anchored Writer fly frames:
    // Last known top of line, in which the anchor character is in.
    // Used to decide, if invalidation has to been performed, if anchor position
    // has changed, and used to position Writer fly frame.
    SwTwips mnLastTopOfLine;

    /** check anchor character rectangle

        OD 11.11.2003 #i22341#
        helper method for method <CheckCharRectAndTopOfLine()>
        For to-character anchored Writer fly frames the member <maLastCharRect>
        is updated. This is checked for change and depending on the applied
        positioning, it's decided, if the Writer fly frame has to be invalidated.

        @author OD

        @param _rAnch
        input parameter - reference to anchor position
    */
    void _CheckCharRect( const SwFmtAnchor& _rAnch );

    /** check top of line

        OD 11.11.2003 #i22341#
        helper method for method <CheckCharRectAndTopOfLine()>
        For to-character anchored Writer fly frames the member <mnLastTopOfLine>
        is updated. This is checked for change and depending on the applied
        positioning, it's decided, if the Writer fly frame has to be invalidated.

        @author OD

        @param _rAnch
        input parameter - reference to anchor position
    */
    void _CheckTopOfLine( const SwFmtAnchor& _rAnch );

protected:
    //Stellt sicher, das der Fly an der richtigen Seite haengt.
    void AssertPage();

    virtual void MakeAll();
    virtual void MakeFlyPos();
public:
    SwFlyAtCntFrm( SwFlyFrmFmt*, SwFrm *pAnchor );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );

    void SetAbsPos( const Point &rNew );

    /** check anchor character rectangle and top of line

        OD 11.11.2003 #i22341#
        For to-character anchored Writer fly frames the members <maLastCharRect>
        and <maLastTopOfLine> are updated. These are checked for change and
        depending on the applied positioning, it's decided, if the Writer fly
        frame has to be invalidated.

        @author OD
    */
    void CheckCharRectAndTopOfLine();

    SwTwips GetLastCharX() const { return maLastCharRect.Left() - Frm().Left(); }

    SwTwips GetRelCharX( const SwFrm* pFrm ) const
        { return maLastCharRect.Left() - pFrm->Frm().Left(); }
    SwTwips GetRelCharY( const SwFrm* pFrm ) const
        { return maLastCharRect.Bottom() - pFrm->Frm().Top(); }

    void AddLastCharY( long nDiff ){ maLastCharRect.Pos().Y() += nDiff; }

    // accessors to data of position calculation
    // frame vertical position is orient at
    inline const SwLayoutFrm* GetVertPosOrientFrm() const
    {
        return mpVertPosOrientFrm;
    }
};

//Die Flys, die an einem Zeichen in einem Cntnt haengen.
class SwFlyInCntFrm : public SwFlyFrm
{
    Point aRef;  //Relativ zu diesem Point wird die AbsPos berechnet.
    long  nLine; //Zeilenhoehe, Ref.Y() - nLine == Zeilenanfang.

    BOOL bInvalidLayout :1;
    BOOL bInvalidCntnt  :1;

    virtual void MakeFlyPos();

protected:
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);
    virtual void MakeAll();

public:
    SwFlyInCntFrm( SwFlyFrmFmt*, SwFrm *pAnchor );

    virtual ~SwFlyInCntFrm();
    virtual void  Format(  const SwBorderAttrs *pAttrs = 0 );
    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );

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

#endif
