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
#ifndef _TABFRM_HXX
#define _TABFRM_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#include "layfrm.hxx"
#include "flowfrm.hxx"
namespace binfilter {

class SwTable;
class SwBorderAttrs;
class SwAttrSetChg;

class SwTabFrm: public SwLayoutFrm, public SwFlowFrm
{
    // OD 14.03.2003 #i11760# - adjustment, because of method signature change
    //darf mit den Flags spielen.
    friend void CalcCntnt( SwLayoutFrm *pLay, bool bNoColl, bool bNoCalcFollow );

    //Fuert Spezialbehandlung fuer _Get[Next|Prev]Leaf() durch.
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, BOOL bFwd );

    SwTable *pTable;

    BOOL bComplete	  		:1;	//Eintrage als Repaint ohne das CompletePaint
                                //der Basisklasse gesetzt werden muss. Damit
                                //sollen unertraegliche Tabellen-Repaints
                                //vermieden werden.
    BOOL bCalcLowers  		:1;	//Im MakeAll auf jedenfall auch fuer Stabilitaet
                                //des Inhaltes sorgen.
    BOOL bLowersFormatted	:1;//Kommunikation zwischen MakeAll und Layact
    BOOL bLockBackMove		:1;	//BackMove-Test hat der Master erledigt.
    BOOL bResizeHTMLTable	:1;	//Resize des HTMLTableLayout rufen im MakeAll
                                //Zur Optimierung, damit dies nicht im
                                //CntntFrm::Grow gerufen werden muss, denn dann
                                //wird es ggf. fuer jede Zelle gerufen #47483#
    BOOL bONECalcLowers		:1;	//Primaer fuer die StarONE-SS. Beim MakeAll werden
                                //die Cntnts auf jedenfall per Calc() formatiert.
                                //es finden keine zusaetzlichen Invalidierungen
                                //statt und dieser Weg kann auch kaum garantien
                                //geben.

    //Split() spaltet den Frm an der angegebenen Stelle, es wird ein
    //Follow erzeugt und aufgebaut und direkt hinter this gepastet.
    //Join() Holt sich den Inhalt aus dem Follow und vernichtet diesen.
    SwTwips Split( const SwTwips nCutPos );
    SwTwips Join();

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual BOOL ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat );

protected:
    virtual void MakeAll();
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
        //Aendert nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
public:
    SwTabFrm( SwTable & );	//Immer nach dem erzeugen _und_ pasten das
                            //Regist Flys rufen!
    SwTabFrm( SwTabFrm & );	//_Nur_ zum erzeugen von Follows
    ~SwTabFrm();


    //Ruft das RegistFlys der Zeilen.
    void RegistFlys();

    inline const SwTabFrm *GetFollow() const;
    inline		 SwTabFrm *GetFollow();
    inline const SwTabFrm *FindMaster() const;
    inline		 SwTabFrm *FindMaster();

    virtual	void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual BOOL GetInfo( SfxPoolItem &rHnt ) const;
    virtual void  CheckDirection( BOOL bVert );

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

                 SwCntntFrm *FindLastCntnt();
    inline const SwCntntFrm *FindLastCntnt() const;

    const SwTable *GetTable() const { return pTable; }
          SwTable *GetTable() 		{ return pTable; }

    BOOL IsComplete()  { return bComplete; }
    void SetComplete() { bComplete = TRUE; }
    void ResetComplete() { bComplete = FALSE; }

    BOOL IsLowersFormatted() const 		{ return bLowersFormatted; }
    void SetLowersFormatted( BOOL b )	{ bLowersFormatted = b;    }

    void SetCalcLowers()		{ bCalcLowers = TRUE; 	   } //Sparsam einsetzen!
    void SetResizeHTMLTable()   { bResizeHTMLTable = TRUE; } //dito
    void SetONECalcLowers()		{ bONECalcLowers = TRUE;   }

    BOOL CalcFlyOffsets( SwTwips& rUpper, long& rLeftOffset,
                         long& rRightOffset ) const;
    DECL_FIXEDMEMPOOL_NEWDEL(SwTabFrm)
};

inline const SwCntntFrm *SwTabFrm::FindLastCntnt() const
{
    return ((SwTabFrm*)this)->FindLastCntnt();
}

inline const SwTabFrm *SwTabFrm::GetFollow() const
{
    return (const SwTabFrm*)SwFlowFrm::GetFollow();
}
inline SwTabFrm *SwTabFrm::GetFollow()
{
    return (SwTabFrm*)SwFlowFrm::GetFollow();
}

inline const SwTabFrm *SwTabFrm::FindMaster() const
{
    return (const SwTabFrm*)SwFlowFrm::FindMaster();
}
inline SwTabFrm *SwTabFrm::FindMaster()
{
    return (SwTabFrm*)SwFlowFrm::FindMaster();
}

} //namespace binfilter
#endif	//_TABFRM_HXX
