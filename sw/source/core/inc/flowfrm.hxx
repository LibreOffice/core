/*************************************************************************
 *
 *  $RCSfile: flowfrm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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
#ifndef _FLOWFRM_HXX
#define _FLOWFRM_HXX

//Der FlowFrm gibt die Funktionalitaet fuer alle Frms vor, die fliessen und
//die sich aufspalten koennen (wie CntntFrm oder TabFrm).
//Teile der Funktionalitaet sind im FlowFrm implementiert, andere Teile werden
//von den spezifischen Frms implementiert.
//Der FlowFrm ist kein eigenstaender Frm, es kann also auch niemals eine
//eigenstaendige Instanz vom FlowFrm existieren.
//Der FlowFrm ist nicht einmal ein echter Frm. Die naheliegende Implementierung
//waere ein FlowFrm der virtual vom SwFrm abgeleitet ist und direkt auf den
//eigenen Instanzdaten arbeitet. Abgeleitete Klassen muessten sich
//vom FlowFrm und (ueber mehrere Basisklassen weil der Klassenbaum sich direkt
//vom SwFrm zu SwCntntFrm und zum SwLayoutFrm spaltet) virtual vom SwFrm
//ableiten.
//Leider entstehen dadurch - neben Problemen mit Compilern und Debuggern -
//erhebliche zusaetzliche Kosten, die wir uns heutzutage IMHO nicht erlauben
//koennen.
//Ich greife deshalb auf eine andere Technik zurueck: Der FlowFrm hat eine
//Referenz auf den SwFrm - der er genau betrachtet selbst ist - und ist mit
//diesem befreundet. So kann der FlowFrm anstelle des this-Pointer mit der
//Referenz auf den SwFrm arbeiten.

#include "frame.hxx"    //fuer inlines

class SwPageFrm;
class SwRect;
class SwBorderAttrs;
class SwDoc;
class SwNodeIndex;

class SwFlowFrm
{
    //PrepareMake darf Locken/Unlocken (Robustheit)
    friend inline void PrepareLock  ( SwFlowFrm * );
    friend inline void PrepareUnlock( SwFlowFrm * );

    //TblSel darf das Follow-Bit zuruecksetzen.
    friend inline void UnsetFollow( SwFlowFrm *pFlow );

    friend void MakeFrms( SwDoc *, const SwNodeIndex &, const SwNodeIndex & );

    friend class SwNode2LayImpl;

    SwFrm &rThis;

    //Hilfsfunktionen fuer MoveSubTree()
    static SwLayoutFrm *CutTree( SwFrm* );
    static BOOL   PasteTree( SwFrm *, SwLayoutFrm *, SwFrm *, SwFrm* );

    //Wird fuer das Zusammenspiel von _GetPrevxxx und MoveBwd gebraucht, damit
    //mehrere Blaetter gleichzeitig uebersprungen werden koennen.
    //Wird auch vom MoveBwd des TabFrm ausgewertet!
    static BOOL bMoveBwdJump;

protected:

    SwFlowFrm *pFollow;

    BOOL bIsFollow  :1; //Ist's ein Follow
    BOOL bLockJoin  :1; //Join (und damit deleten) verboten wenn TRUE!
    BOOL bUndersized:1; // wir sind kleiner als gewuenscht
    BOOL bFtnAtEnd  :1; // For sectionfrms only: footnotes at the end of section
    BOOL bEndnAtEnd :1; //  "       "        " : endnotes at the end of section
    BOOL bCntntLock :1; //  "       "        " : content locked
    BOOL bOwnFtnNum :1; //  "       "        " : special numbering of footnotes
    BOOL bFtnLock   :1; //  "       "        " : ftn, don't leave this section bwd

    //Prueft ob Vorwaertsfluss noch Sinn macht Endloswanderschaften (unterbinden)
    inline BOOL IsFwdMoveAllowed();
    BOOL IsKeepFwdMoveAllowed();    //Wie oben, Move fuer Keep.

    //Prueft ob ein Obj das Umlauf wuenscht ueberlappt.
    //eine Null bedeutet, kein Objekt ueberlappt,
    // 1 heisst, Objekte, die am FlowFrm selbst verankert sind, ueberlappen
    // 2 heisst, Objekte, die woanders verankert sind, ueberlappen
    // 3 heistt, beiderlei verankerte Objekte ueberlappen
    BYTE BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect );

    void LockJoin()   { bLockJoin = TRUE;  }
    void UnlockJoin() { bLockJoin = FALSE; }

            BOOL CheckMoveFwd( BOOL &rbMakePage, BOOL bKeep, BOOL bMovedBwd );
            BOOL MoveFwd( BOOL bMakePage, BOOL bPageBreak, BOOL bMoveAlways = FALSE );
    virtual BOOL ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat )=0;
            BOOL MoveBwd( BOOL &rbReformat );

    BOOL IsKeep( const SwBorderAttrs &rAttrs ) const; //Ist ein Keep zu beruecksichtigen (Breaks!)

public:
    SwFlowFrm( SwFrm &rFrm );

    const SwFrm *GetFrm() const            { return &rThis; }
          SwFrm *GetFrm()                  { return &rThis; }

    static BOOL IsMoveBwdJump()            { return bMoveBwdJump; }
    static void SetMoveBwdJump( BOOL bNew ){ bMoveBwdJump = bNew; }

    inline void SetUndersized( const BOOL bNew ) { bUndersized = bNew; }
    inline BOOL IsUndersized()  const { return bUndersized; }

    BOOL IsPrevObjMove() const;

    //Die Kette mit minimalen Operationen und Benachrichtigungen unter den
    //neuen Parent Moven.
    void MoveSubTree( SwLayoutFrm* pParent, SwFrm* pSibling = 0 );

           BOOL       HasFollow() const    { return pFollow ? TRUE : FALSE; }
           BOOL       IsFollow()     const { return bIsFollow; }
    inline void       _SetIsFollow( BOOL bSet ) { bIsFollow = bSet; }
    const  SwFlowFrm *GetFollow() const    { return pFollow;   }
           SwFlowFrm *GetFollow()          { return pFollow;   }
           BOOL       IsAnFollow( const SwFlowFrm *pFlow ) const;
    inline void       SetFollow( SwFlowFrm *pNew ) { pFollow = pNew; }
                 SwFlowFrm *FindMaster();
    inline const SwFlowFrm *FindMaster() const;

    BOOL IsJoinLocked() const { return bLockJoin; }
    BOOL IsFtnAtEnd() const { return bFtnAtEnd; }
    BOOL IsEndnAtEnd() const { return bEndnAtEnd;   }
    BOOL IsAnyNoteAtEnd() const { return bFtnAtEnd || bEndnAtEnd; }
    BOOL AreNotesAtEnd() const { return bFtnAtEnd && bEndnAtEnd; }

    BOOL IsPageBreak( BOOL bAct ) const;
    BOOL IsColBreak( BOOL bAct ) const;

    BOOL HasParaSpaceAtPages( BOOL bSct ) const;
    SwTwips CalcUpperSpace( const SwBorderAttrs *pAttrs = NULL,
                            const SwFrm* pPr = NULL ) const;

    void CheckKeep();

    void SetFtnLock( BOOL bNew ){ bFtnLock = bNew; }
    BOOL IsFtnLock() const {    return bFtnLock; }
    void SetOwnFtnNum( BOOL bNew ){ bOwnFtnNum = bNew; }
    BOOL IsOwnFtnNum() const {  return bOwnFtnNum; }
    void SetCntntLock( BOOL bNew ){ bCntntLock = bNew; }
    BOOL IsCntntLocked() const {    return bCntntLock; }

    //casten einen Frm auf einen FlowFrm - wenns denn einer ist, sonst 0
    //Diese Methoden muessen fuer neue Ableitungen geaendert werden!
    static       SwFlowFrm *CastFlowFrm( SwFrm *pFrm );
    static const SwFlowFrm *CastFlowFrm( const SwFrm *pFrm );
};

inline const SwFlowFrm *SwFlowFrm::FindMaster() const
{
    return ((SwFlowFrm*)this)->FindMaster();
}
inline BOOL SwFlowFrm::IsFwdMoveAllowed()
{
    return rThis.GetIndPrev() != 0;
}


#endif
