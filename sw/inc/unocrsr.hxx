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
#ifndef INCLUDED_SW_INC_UNOCRSR_HXX
#define INCLUDED_SW_INC_UNOCRSR_HXX
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
        sal_Bool & io_rbLeft, sal_Bool bVisualAllowed, sal_Bool bInsertCrsr) SAL_OVERRIDE;
    virtual void DoSetBidiLevelUpDown() SAL_OVERRIDE;

public:

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
    virtual sal_Bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS )) SAL_OVERRIDE;

    virtual bool IsReadOnlyAvailable() const SAL_OVERRIDE;

    sal_Bool IsRemainInSection() const          { return bRemainInSection; }
    void SetRemainInSection( sal_Bool bFlag )   { bRemainInSection = bFlag; }

    virtual sal_Bool IsSkipOverProtectSections() const SAL_OVERRIDE
                                    { return bSkipOverProtectSections; }
    void SetSkipOverProtectSections( sal_Bool bFlag )
                                    { bSkipOverProtectSections = bFlag; }

    virtual sal_Bool IsSkipOverHiddenSections() const SAL_OVERRIDE
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
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS )) SAL_OVERRIDE;

    virtual SwUnoTableCrsr * Clone() const SAL_OVERRIDE;

    void MakeBoxSels();

          SwCursor& GetSelRing()            { return aTblSel; }
    const SwCursor& GetSelRing() const      { return aTblSel; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
