/*************************************************************************
 *
 *  $RCSfile: ndhints.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-06 18:52:24 $
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
#ifndef _NDHINTS_HXX
#define _NDHINTS_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "numrule.hxx"

class SwTxtNode;
class SwRegHistory;                 // steht im RolBck.hxx
class SwTxtAttr;

/*
 * Ableitung der Klasse SwpHints ueber den Umweg ueber SwpHts, da
 * lediglich die Klasse SwTxtNode Attribute einfuegen und
 * loeschen koennen soll. Anderen Klassen wie den Frames steht
 * lediglich ein lesender Zugriff ueber den Index-Operator zur
 * Verfuegung.
 * Groesse beim Anlegen gleich 1, weil nur dann ein Array erzeug wird, wenn
 * auch ein Hint eingefuegt wird.
 */

/*************************************************************************
 *                      class SwpHtStart/End
 *************************************************************************/

SV_DECL_PTRARR_SORT(SwpHtStart,SwTxtAttr*,1,1)
SV_DECL_PTRARR_SORT(SwpHtEnd,SwTxtAttr*,1,1)

/*************************************************************************
 *                      class SwpHintsArr
 *************************************************************************/

// Das neue Hintsarray:
class SwpHintsArr : private SwpHtStart
{

protected:
    SwpHtEnd aHtEnd;

public:
    void Insert( const SwTxtAttr *pHt );
    void Delete( const SwTxtAttr *pHt );
    void DeleteAtPos( const USHORT nPosInStart );
    BOOL Resort();
    SwTxtAttr *Cut( const USHORT nPosInStart );

    inline const SwTxtAttr *GetStart( const USHORT nPos ) const { return (*this)[nPos]; }
    inline const SwTxtAttr *GetEnd( const USHORT nPos ) const { return aHtEnd[nPos]; }
    inline SwTxtAttr *GetStart( const USHORT nPos ) { return GetHt(nPos); }
    inline SwTxtAttr *GetEnd( const USHORT nPos ) { return aHtEnd[nPos]; }
    inline USHORT GetEndCount() const { return aHtEnd.Count(); }
    inline USHORT GetStartCount() const { return Count(); }
    inline USHORT GetStartOf( const SwTxtAttr *pHt ) const;
    inline USHORT GetEndOf( const SwTxtAttr *pHt ) const;
    inline USHORT GetPos( const SwTxtAttr *pHt ) const
//  OS: in svmem.hxx wird fuer TCPP GetPos ohne const gerufen
#ifdef TCPP
        { return SwpHtStart::GetPos( (SwTxtAttr *)pHt ); }
#else
        { return SwpHtStart::GetPos( pHt ); }
#endif

#ifdef USED
    // Der Zugriffsoperator soll bald nur noch const sein!
    inline SwTxtAttr *operator[]( const USHORT nIdx ) { return SwpHtStart::operator[](nIdx); }
#endif


    inline SwTxtAttr *GetHt( const USHORT nIdx ) { return SwpHtStart::operator[](nIdx); }
    inline const SwTxtAttr *operator[]( const USHORT nIdx )const
        { return SwpHtStart::operator[](nIdx); }
    inline USHORT Count() const { return SwpHtStart::Count(); }

    // Methoden vom PtrArr weiterreichen, wird fuer Node::_Cut benoetigt
    const SwTxtAttr** GetData() const  { return SwpHtStart::GetData(); }

#ifndef PRODUCT
    BOOL Check() const;
#endif
};

/*************************************************************************
 *                      class SwpHints
 *************************************************************************/

// Die public-Schnittstelle zum Node hin
class SwpHints: public SwpHintsArr
{
private:
    SwRegHistory* pHistory;
    // Numerierung
    BOOL    bVis    :1;  // HiddenParaFld
    BOOL    bFntChg :1;  // Fontwechsel
    BOOL    bFtn    :1;  // Fussnoten
    BOOL    bInSplitNode: 1;    // TRUE: der Node ist im Split und die Frames
                                // werden verschoben!
    BOOL    bDDEFlds : 1;       // es sind DDE-Felder am TextNode vorhanden
    BOOL    bCalcVis : 1; // bVis ist invalid, CalcVisibleFlag() rufen

    BOOL Resort( const USHORT nPos );
    // loescht Hints, die keinen Zustaendigkeitsbereich mehr haben
    void ClearDummies( SwTxtNode &rNode );
    // Forget signalisiert, dass es ueberfluessig ist, das Attribut einzu-
    // fuegen, da es keinen Zustaendigkeitsbereich haette.
    BOOL Forget( const USHORT i, const USHORT nWhich,
                 const xub_StrLen nStrt, const xub_StrLen nEnd );
    // Merge verschmilzt aneinanderreichende Hints mit gleichen Attributen
    // und gleichen Werten, falls nichts dagegen spricht, d.h. wenn dadurch
    // keine Ueberlappungen mit gleichartigen Attr. entstehen,
    // Ist der Rueckgabewert TRUE, so wurde verschmolzen und dabei u.a. auch
    // ein Resort ausgeloest.
    BOOL Merge( SwTxtNode &rNode );
    // Haelt ein neues Attribut in pHistory fest.
    void NoteInHistory( SwTxtAttr *pAttr, const BOOL bNew = FALSE );

    void CalcFlags( );

    // die Delete Methoden duerfen nur vom TextNode gerufen werden !!
    // Dieser sorgt dafuer, das bei Attributen ohne Ende auch das
    // entsp. Zeichen entfernt wird !!
    friend class SwTxtNode;
    void DeleteAtPos( const USHORT nPos );
    // Ist der Hint schon bekannt, dann suche die Position und loesche ihn.
    // Ist er nicht im Array, so gibt es ein ASSERT !!
    void Delete( SwTxtAttr* pTxtHt );

public:
    inline BOOL CanBeDeleted() const { return !Count(); }

    // damit das SwDoc::Undo ueber alle Attributaenderungen informiert
    // wird, gibt es hier einen Pointer auf die History. Nur wenn dieser
    // != 0 ist, muessen alle Attributaenderungen "gemeldet" werden.
    void Register( SwRegHistory* pHist ) { pHistory = pHist; }
    void DeRegister() { Register(0); }

    void Insert( SwTxtAttr *pHt, SwTxtNode &rNode, USHORT nMode = 0 );

    inline void SetCalcVisible(){ bCalcVis = TRUE; }
    inline void SetVisible( const BOOL bNew )  { bVis = bNew; }
    inline BOOL IsVisible() const
        { if( bCalcVis ) ((SwpHints*)this)->CalcVisibleFlag(); return bVis; }
    inline BOOL HasFntChg() const { return bFntChg; }
    inline BOOL HasFtn() const { return bFtn; }
    inline BOOL IsInSplitNode() const { return bInSplitNode; }
    inline BOOL HasDDEFld() const { return bDDEFlds; }

    // Konstruktor  (wird im nur einmal benutzt!)
    SwpHints()
    {
        pHistory = 0;
        bFntChg = bVis = TRUE;
        bDDEFlds = bFtn = bInSplitNode = bCalcVis = FALSE;
    }

    // Berechnung von bVis, return-Wert TRUE bei Aenderung
    BOOL CalcVisibleFlag();

    DECL_FIXEDMEMPOOL_NEWDEL(SwpHints)
};

// Ausgabeoperator fuer die Texthints
SvStream &operator<<(SvStream &aS, const SwpHints &rHints); //$ ostream

/*************************************************************************
 *                       Inline-Implementationen
 *************************************************************************/

inline USHORT SwpHintsArr::GetStartOf( const SwTxtAttr *pHt ) const
{
    USHORT nPos;
    if( !Seek_Entry( pHt, &nPos ) )
        nPos = USHRT_MAX;
    return nPos;
}

inline USHORT SwpHintsArr::GetEndOf( const SwTxtAttr *pHt ) const
{
    USHORT nPos;
    if( !aHtEnd.Seek_Entry( pHt, &nPos ) )
        nPos = USHRT_MAX;
    return nPos;
}

inline SwTxtAttr *SwpHintsArr::Cut( const USHORT nPosInStart )
{
    SwTxtAttr *pHt = GetHt(nPosInStart);
    DeleteAtPos( nPosInStart );
    return pHt;
}


#endif
