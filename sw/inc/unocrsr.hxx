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
#ifndef _UNOCRSR_HXX
#define _UNOCRSR_HXX
#include <swcrsr.hxx>
#include <calbck.hxx>


class SwUnoCrsr : public virtual SwCursor, public SwModify
{
    sal_Bool bRemainInSection : 1;
    sal_Bool bSkipOverHiddenSections : 1;
    sal_Bool bSkipOverProtectSections : 1;

public:
    SwUnoCrsr( const SwPosition &rPos, SwPaM* pRing = 0 );
    virtual ~SwUnoCrsr();

    // @@@ semantic: no copy ctor.
    SwUnoCrsr( SwUnoCrsr& );
private:
    // forbidden and not implemented.
    //SwUnoCrsr( const SwUnoCrsr& );
    SwUnoCrsr & operator= ( const SwUnoCrsr& );

protected:

    virtual const SwCntntFrm* DoSetBidiLevelLeftRight(
        sal_Bool & io_rbLeft, sal_Bool bVisualAllowed, sal_Bool bInsertCrsr);
    virtual void DoSetBidiLevelUpDown();

public:

//  virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    // gibt es eine Selection vom Content in die Tabelle
    // Return Wert gibt an, ob der Crsr auf der alten Position verbleibt
    virtual sal_Bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));

    virtual bool IsReadOnlyAvailable() const;

    sal_Bool IsRemainInSection() const          { return bRemainInSection; }
    void SetRemainInSection( sal_Bool bFlag )   { bRemainInSection = bFlag; }

    virtual sal_Bool IsSkipOverProtectSections() const
                                    { return bSkipOverProtectSections; }
    void SetSkipOverProtectSections( sal_Bool bFlag )
                                    { bSkipOverProtectSections = bFlag; }

    virtual sal_Bool IsSkipOverHiddenSections() const
                                    { return bSkipOverHiddenSections; }
    void SetSkipOverHiddenSections( sal_Bool bFlag )
                                    { bSkipOverHiddenSections = bFlag; }

    // make copy of cursor
    virtual SwUnoCrsr * Clone() const;

    DECL_FIXEDMEMPOOL_NEWDEL( SwUnoCrsr )
};



class SwUnoTableCrsr : public virtual SwUnoCrsr, public virtual SwTableCursor
{
    // die Selection hat die gleiche Reihenfolge wie die
    // TabellenBoxen. D.h., wird aus dem einen Array an einer Position
    // etwas geloescht, dann muss es auch im anderen erfolgen!!
    SwCursor aTblSel;

    using SwTableCursor::MakeBoxSels;

public:
    SwUnoTableCrsr( const SwPosition& rPos );
    virtual ~SwUnoTableCrsr();

//  virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    // gibt es eine Selection vom Content in die Tabelle
    // Return Wert gibt an, ob der Crsr auf der alten Position verbleibt
    virtual sal_Bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));

    virtual SwUnoTableCrsr * Clone() const;

    void MakeBoxSels();

          SwCursor& GetSelRing()            { return aTblSel; }
    const SwCursor& GetSelRing() const      { return aTblSel; }
};



#endif
