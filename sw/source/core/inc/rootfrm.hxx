/*************************************************************************
 *
 *  $RCSfile: rootfrm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _ROOTFRM_HXX
#define _ROOTFRM_HXX

#include "layfrm.hxx"

class SwCntntFrm;
class ViewShell;
class SdrPage;
class SwFrmFmt;
class SwPaM;
class SwCursor;
class SwShellCrsr;
class SwTableCursor;
class SwRegionRects;
class Sw3FrameIo;
class OutputDevice;
class SwLayVout;
class SwDestroyList;
class SwCurrShells;
struct SwPosition;
struct SwCrsrMoveState;

#define HACK_TABLEMODE_INIT         0
#define HACK_TABLEMODE_LOCKLINES    1
#define HACK_TABLEMODE_PAINTLINES   2
#define HACK_TABLEMODE_UNLOCKLINES  3
#define HACK_TABLEMODE_EXIT         4

#define INV_SIZE    1
#define INV_PRTAREA 2
#define INV_POS     4
#define INV_TABLE   8
#define INV_SECTION 16
#define INV_LINENUM 32

class SwRootFrm: public SwLayoutFrm
{
    //Muss das Superfluous temporaer abschalten.
    friend void AdjustSizeChgNotify( SwRootFrm *pRoot );

    //Pflegt pLastPage (Cut() und Paste() vom SwPageFrm
    friend inline void SetLastPage( SwPageFrm* );

    // Fuer das Anlegen und Zerstoeren des virtuellen Outputdevice-Managers
    friend void _FrmInit();     //erzeugt pVout
    friend void _FrmFinit();    //loescht pVout

    static SwLayVout     *pVout;
    static BOOL           bInPaint;     //Schutz gegen doppelte Paints.
    static BOOL           bNoVirDev;    //Bei SystemPaints kein virt. Device

    BOOL    bCheckSuperfluous   :1; //Leere Seiten suchen?
    BOOL    bIdleFormat         :1; //Idle-Formatierer anwerfen?
    BOOL    bBrowseWidthValid   :1; //Ist nBrowseWidth gueltig?
    BOOL    bDummy2             :1; //Unbenutzt
    BOOL    bTurboAllowed       :1;
    BOOL    bAssertFlyPages     :1; //Ggf. weitere Seiten fuer Flys einfuegen?
    BOOL    bDummy              :1; //Unbenutzt
    BOOL    bIsVirtPageNum      :1; //gibt es eine virtuelle Seitennummer ?
    BOOL    bIsNewLayout        :1; //Layout geladen oder neu erzeugt.
    BOOL    bCallbackActionEnabled:1; //Keine Action in Benachrichtung erwuenscht
                                    //siehe dcontact.cxx, ::Changed()

    //Fuer den BrowseMode. nBrowseWidth ist die Aeussere Kante des am weitesten
    //rechts stehenden Objectes. Die rechte Kante der Seiten soll im BrowseMode
    //nicht kleiner werden als dieser Wert.
    long    nBrowseWidth;

    //Wenn nur _ein: CntntFrm zu formatieren ist, so steht dieser in pTurbo.
    const SwCntntFrm *pTurbo;

    //Die letzte Seite wollen wir uns nicht immer muehsam zusammensuchen.
    SwPageFrm *pLastPage;

    //Die Root kuemmert sich nun auch um den Shell-Zugriff. Ueber das Dokument
    //sollte man auch immer an die Root herankommen und somit auch immer
    //einen Zugriff auf die Shell haben.
    //Der Pointer pCurrShell ist der Pointer auf irgendeine der Shells fuer
    //das Dokument
    //Da es durchaus nicht immer egal ist, auf welcher Shell gearbeitet wird,
    //ist es notwendig die aktive Shell zu kennen. Das wird dadurch angenaehert,
    //dass der Pointer pCurrShell immer dann umgesetzt wird, wenn eine
    //Shell den Fokus erhaelt (FEShell). Zusaetzlich wird der Pointer
    //Temporaer von SwCurrShell umgesetzt, dieses wird typischerweise
    //ueber das Macro SET_CURR_SHELL erledigt. Makro + Klasse sind in der
    //ViewShell zu finden. Diese Objekte koennen auch verschachtelt (auch fuer
    //unterschiedliche Shells) erzeugt werden. Sie werden im Array pCurrShells
    //gesammelt.
    //Weiterhin kann es noch vorkommen, dass eine Shell aktiviert wird,
    //waehrend noch ein CurrShell-Objekt "aktiv" ist. Dieses wird dann in
    //pWaitingCurrShell eingetragen und vom letzten DTor der CurrShell
    //"aktiviert".
    //Ein weiteres Problem ist dass Zerstoeren einer Shell waehrend sie aktiv
    //ist. Der Pointer pCurrShell wird dann auf eine beliebige andere Shell
    //umgesetzt.
    //Wenn zum Zeitpunkt der zerstoerung einer Shell diese noch in irgendwelchen
    //CurrShell-Objekten referenziert wird, so wird auch dies aufgeklart.
    friend class CurrShell;
    friend void SetShell( ViewShell *pSh );
    friend void InitCurrShells( SwRootFrm *pRoot );
    ViewShell *pCurrShell;
    ViewShell *pWaitingCurrShell;
    SwCurrShells *pCurrShells;

    //Eine Page im DrawModel pro Dokument, hat immer die Groesse der Root.
    SdrPage *pDrawPage;

    SwDestroyList* pDestroy;

    USHORT  nPhyPageNums;           //Anzahl der Seiten.

    void ImplCalcBrowseWidth();
    void ImplInvalidateBrowseWidth();

    void _DeleteEmptySct(); // zerstoert ggf. die angemeldeten SectionFrms
    void _RemoveFromList( SwSectionFrm* pSct ); // entfernt SectionFrms aus der Delete-Liste

protected:

    virtual void MakeAll();

public:

    //MasterObjekte aus der Page entfernen (von den Ctoren gerufen).
    static void RemoveMasterObjs( SdrPage *pPg );

    //Virtuelles Device ausgeben (z.B. wenn Animationen ins Spiel kommen)
    static BOOL FlushVout();
    //Clipping sparen, wenn im Vout eh genau das Cliprechteck ausgegeben wird
    static BOOL HasSameRect( const SwRect& rRect );

    SwRootFrm( SwFrmFmt*, ViewShell* );
    SwRootFrm( Sw3FrameIo&, SwLayoutFrm* );
    ~SwRootFrm();

    ViewShell *GetCurrShell() const { return pCurrShell; }
    void SwRootFrm::DeRegisterShell( ViewShell *pSh );

    //Start-/EndAction fuer alle Shells auf moeglichst hoeher
    //(Shell-Ableitungs-)Ebene aufsetzen. Fuer die StarONE Anbindung, die
    //die Shells nicht dirkt kennt.
    //Der ChangeLinkd der CrsrShell (UI-Benachrichtigung) wird im EndAllAction
    //automatisch gecallt.
    void StartAllAction();
    void EndAllAction( BOOL bVirDev = FALSE );

    // fuer bestimmte UNO-Aktionen (Tabellencursor) ist es notwendig, dass alle Actions
    // kurzfristig zurueckgesetzt werden. Dazu muss sich jede ViewShell ihren alten Action-zaehler
    // merken
    void UnoRemoveAllActions();
    void UnoRestoreAllActions();

    const SdrPage* GetDrawPage() const { return pDrawPage; }
          SdrPage* GetDrawPage()       { return pDrawPage; }
          void     SetDrawPage( SdrPage* pNew ){ pDrawPage = pNew; }

    virtual void  Store( Sw3FrameIo& ) const;
    virtual BOOL  GetCrsrOfst( SwPosition *, Point&,
                               const SwCrsrMoveState* = 0 ) const;
    virtual void  Paint( const SwRect& ) const;
    virtual SwTwips ShrinkFrm( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
#ifndef PRODUCT
    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    Point  GetNextCntntPos( const Point &rPoint, BOOL bIgnoreTblHdln ) const;
    Point  GetPrevCntntPos( const Point &rPoint ) const;
    void   SetFixSize( SzPtr );
    virtual void ChgSize( const Size& aNewSize );

    inline void SetIdleFlags();
           BOOL IsIdleFormat()  const { return bIdleFormat; }
           void ResetIdleFormat()     { bIdleFormat = FALSE; }

    //Sorgt dafuer, dass alle gewuenschten Seitengebunden Flys eine Seite finden
    void SetAssertFlyPages() { bAssertFlyPages = TRUE; }
    void AssertFlyPages();
    BOOL IsAssertFlyPages()  { return bAssertFlyPages; }

    //Stellt sicher, dass ab der uebergebenen Seite auf allen Seiten die
    //Seitengebundenen Rahmen auf der richtigen Seite (Seitennummer) stehen.
    void AssertPageFlys( SwPageFrm * );

    //Saemtlichen Inhalt invalidieren, Size oder PrtArea
    void InvalidateAllCntnt( BYTE nInvalidate = INV_SIZE );

    //Ueberfluessige Seiten entfernen.
    void SetSuperfluous()      { bCheckSuperfluous = TRUE; }
    BOOL IsSuperfluous() const { return bCheckSuperfluous; }
    void RemoveSuperfluous();

    //abfragen/setzen der aktuellen Seite und der Gesamtzahl der Seiten.
    //Es wird soweit wie notwendig Formatiert.
    USHORT  GetCurrPage( const SwPaM* ) const;
    USHORT  SetCurrPage( SwCursor*, USHORT nPageNum );
    Point   GetPagePos( USHORT nPageNum ) const;
    USHORT  GetPageNum() const      { return nPhyPageNums; }
    void    DecrPhyPageNums()       { --nPhyPageNums; }
    void    IncrPhyPageNums()       { ++nPhyPageNums; }
    BOOL    IsVirtPageNum() const   { return bIsVirtPageNum; }
    inline  void SetVirtPageNum( const BOOL bOf ) const;

    //Der Crsr moechte die zu selektierenden Bereiche wissen.
    void CalcFrmRects( SwShellCrsr&, BOOL bIsTblSel );
    //Ermitteln der von der Selection eingeschl. Zellen.
    void MakeTblCrsrs( SwTableCursor& );

    void DisallowTurbo()  const { ((SwRootFrm*)this)->bTurboAllowed = FALSE; }
    void ResetTurboFlag() const { ((SwRootFrm*)this)->bTurboAllowed = TRUE; }
    BOOL IsTurboAllowed() const { return bTurboAllowed; }
    void SetTurbo( const SwCntntFrm *pCntnt ) { pTurbo = pCntnt; }
    void ResetTurbo() { pTurbo = 0; }
    const SwCntntFrm *GetTurbo() { return pTurbo; }

    //Fussnotennummern aller Seiten auf den neuesten Stand bringen.
    void UpdateFtnNums();           //nur bei Seitenweiser Nummerierung!

    //Alle Fussnoten (nicht etwa die Referenzen) entfernen.
    void RemoveFtns( SwPageFrm *pPage = 0, BOOL bPageOnly = FALSE,
                     BOOL bEndNotes = FALSE );
    void CheckFtnPageDescs( BOOL bEndNote );

    const SwPageFrm *GetLastPage() const { return pLastPage; }
          SwPageFrm *GetLastPage()       { return pLastPage; }

    static BOOL IsInPaint() { return bInPaint; }

    static void SetNoVirDev( const BOOL bNew ) { bNoVirDev = bNew; }

    inline long GetBrowseWidth() const;
    void SetBrowseWidth( long n ) { bBrowseWidthValid = TRUE; nBrowseWidth = n;}
    inline void InvalidateBrowseWidth();

    void HackPrepareLongTblPaint( int nMode );

    BOOL IsNewLayout() const { return bIsNewLayout; }
    void ResetNewLayout()    { bIsNewLayout = FALSE;}

    // Hier werden leere SwSectionFrms zur Zerstoerung angemeldet
    // und spaeter zerstoert oder wieder abgemeldet
    void InsertEmptySct( SwSectionFrm* pDel );
    void DeleteEmptySct() { if( pDestroy ) _DeleteEmptySct(); }
    void RemoveFromList( SwSectionFrm* pSct ) { if( pDestroy ) _RemoveFromList( pSct ); }
#ifndef PRODUCT
    // Wird zur Zeit nur fuer ASSERTs benutzt:
    BOOL IsInDelList( SwSectionFrm* pSct ) const; // Ist der SectionFrm in der Liste enthalten?
#endif


    void SetCallbackActionEnabled( BOOL b ) { bCallbackActionEnabled = b; }
    BOOL IsCallbackActionEnabled() const    { return bCallbackActionEnabled; }
};

inline long SwRootFrm::GetBrowseWidth() const
{
    if ( !bBrowseWidthValid )
        ((SwRootFrm*)this)->ImplCalcBrowseWidth();
    return nBrowseWidth;
}

inline void SwRootFrm::InvalidateBrowseWidth()
{
    if ( bBrowseWidthValid )
        ImplInvalidateBrowseWidth();
}

inline void SwRootFrm::SetIdleFlags()
{
    bIdleFormat = TRUE;
}

inline  void SwRootFrm::SetVirtPageNum( const BOOL bOf) const
{
    ((SwRootFrm*)this)->bIsVirtPageNum = bOf;
}

#endif  //_ROOTFRM_HXX

