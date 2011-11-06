/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
