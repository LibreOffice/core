/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX
#include <tools/mempool.hxx>

#include "layfrm.hxx"
#include "flowfrm.hxx"
#include <set>

class SwSection;
class SwSectionFmt;
class SwAttrSetChg;
class SwFtnContFrm;
class SwLayouter;

#define FINDMODE_ENDNOTE 1
#define FINDMODE_LASTCNT 2
#define FINDMODE_MYLAST  4

class SwSectionFrm: public SwLayoutFrm, public SwFlowFrm
{
    SwSection* pSection;
    bool bFtnAtEnd; // footnotes at the end of section
    bool bEndnAtEnd; // endnotes at the end of section
    bool bCntntLock; // content locked
    bool bOwnFtnNum; // special numbering of footnotes
    bool bFtnLock; // ftn, don't leave this section bwd

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );
    void _Cut( sal_Bool bRemove );
    // Is there a FtnContainer?
    // An empty sectionfrm without FtnCont is superfluous
    sal_Bool IsSuperfluous() const { return !ContainsAny() && !ContainsFtnCont(); }
    void CalcFtnAtEndFlag();
    void CalcEndAtEndFlag();
    const SwSectionFmt* _GetEndSectFmt() const;
    bool IsEndnoteAtMyEnd() const;
protected:
    virtual void MakeAll() SAL_OVERRIDE;
    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool bHead, bool &rReformat ) SAL_OVERRIDE;
    virtual void Format( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) SAL_OVERRIDE;

public:
    SwSectionFrm( SwSection &, SwFrm* );                 //Inhalt wird nicht erzeugt!
    SwSectionFrm( SwSectionFrm &, sal_Bool bMaster );//_Nur_ zum Erzeugen von Master/Follows
    virtual ~SwSectionFrm();

    void Init();
    virtual void CheckDirection( bool bVert ) SAL_OVERRIDE;

    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const SAL_OVERRIDE;

    virtual void Cut() SAL_OVERRIDE;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;

    inline const SwSectionFrm *GetFollow() const;
    inline       SwSectionFrm *GetFollow();
    SwSectionFrm* FindMaster() const;

                 SwCntntFrm *FindLastCntnt( sal_uInt8 nMode = 0 );
    inline const SwCntntFrm *FindLastCntnt( sal_uInt8 nMode = 0 ) const;
    inline SwSection* GetSection() { return pSection; }
    inline const SwSection* GetSection() const { return pSection; }
    inline void ColLock()       { mbColLocked = sal_True; }
    inline void ColUnlock()     { mbColLocked = sal_False; }

    void CalcFtnCntnt();
    void SimpleFormat();
    sal_Bool IsDescendantFrom( const SwSectionFmt* pSect ) const;
    sal_Bool HasToBreak( const SwFrm* pFrm ) const;
    void MergeNext( SwSectionFrm* pNxt );
    //Zerlegt den pFrm umgebenden SectionFrm in zwei Teile,
    //pFrm an den Anfang des 2. Teils
    sal_Bool SplitSect( SwFrm* pFrm, sal_Bool bApres );
    void DelEmpty( sal_Bool bRemove );  // wie Cut(), Follow-Verkettung wird aber mitgepflegt
    sal_Bool IsToIgnore() const         // Keine Groesse, kein Inhalt, muss ignoriert werden
    { return !Frm().Height() && !ContainsCntnt(); }
    SwFtnContFrm* ContainsFtnCont( const SwFtnContFrm* pCont = NULL ) const;
    sal_Bool Growable() const;
    SwTwips _Shrink( SwTwips, sal_Bool bTst );
    SwTwips _Grow  ( SwTwips, sal_Bool bTst );

    // A sectionfrm has to maximize, if he has a follow or a ftncontainer at
    // the end of the page. A superfluous follow will be ignored,
    // if bCheckFollow is set.
    sal_Bool ToMaximize( sal_Bool bCheckFollow ) const;
    inline sal_Bool _ToMaximize() const
        { if( !pSection ) return sal_False; return ToMaximize( sal_False ); }
    sal_Bool MoveAllowed( const SwFrm* ) const;
    sal_Bool CalcMinDiff( SwTwips& rMinDiff ) const;
    // Uebergibt man kein bOverSize bzw. sal_False, so ist der Returnwert
    // >0 fuer Undersized-Frames, ==0 sonst.
    // Uebergibt man sal_True, so kann es auch einen negativen Returnwert geben,
    // wenn der SectionFrm nicht vollstaendig ausgefuellt ist, was z.B. bei
    // SectionFrm mit Follow meist vorkommt. Benoetigt wird dies im
    // FormatWidthCols, um "Luft" aus den Spalten zu lassen.
    long Undersize( sal_Bool bOverSize = sal_False );
    // Groesse an die Umgebung anpassen
    void _CheckClipping( sal_Bool bGrow, sal_Bool bMaximize );

    void InvalidateFtnPos();
    void CollectEndnotes( SwLayouter* pLayouter );
    const SwSectionFmt* GetEndSectFmt() const
        { if( IsEndnAtEnd() ) return _GetEndSectFmt(); return NULL; }

    static void MoveCntntAndDelete( SwSectionFrm* pDel, sal_Bool bSave );

    bool IsBalancedSection() const;

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) SAL_OVERRIDE;

    bool IsFtnAtEnd() const { return bFtnAtEnd; }
    bool IsEndnAtEnd() const { return bEndnAtEnd;   }
    bool IsAnyNoteAtEnd() const { return bFtnAtEnd || bEndnAtEnd; }
    bool AreNotesAtEnd() const { return bFtnAtEnd && bEndnAtEnd; }

    void SetCntntLock( bool bNew ) { bCntntLock = bNew; }
    bool IsCntntLocked() const { return bCntntLock; }

    bool IsOwnFtnNum() const { return bOwnFtnNum; }

    void SetFtnLock( bool bNew ) { bFtnLock = bNew; }
    bool IsFtnLock() const { return bFtnLock; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwSectionFrm)
};

class SwDestroyList : public std::set<SwSectionFrm*> {};

inline const SwSectionFrm *SwSectionFrm::GetFollow() const
{
    return (const SwSectionFrm*)SwFlowFrm::GetFollow();
}
inline SwSectionFrm *SwSectionFrm::GetFollow()
{
    return (SwSectionFrm*)SwFlowFrm::GetFollow();
}
inline const SwCntntFrm *SwSectionFrm::FindLastCntnt( sal_uInt8 nMode ) const
{
    return ((SwSectionFrm*)this)->FindLastCntnt( nMode );
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
