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

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
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
    // The selection has the same order as the table boxes, i.e.
    // if something is deleted from the one array at a certain position
    // it has also to be deleted from the other!
    SwCursor aTblSel;

    using SwTableCursor::MakeBoxSels;

public:
    SwUnoTableCrsr( const SwPosition& rPos );
    virtual ~SwUnoTableCrsr();

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
