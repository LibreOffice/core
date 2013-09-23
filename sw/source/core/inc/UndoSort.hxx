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

#ifndef SW_UNDO_SORT_HXX
#define SW_UNDO_SORT_HXX

#include <undobj.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <tools/string.hxx>

struct SwSortOptions;
class SwTableNode;
class SwUndoAttrTbl;

//-------------------- Undo for Sorting ------------------------------
struct SwSortUndoElement
{
    union {
        struct {
            sal_uLong nKenn;
            sal_uLong nSource, nTarget;
        } TXT;
        struct {
            String *pSource, *pTarget;
        } TBL;
    } SORT_TXT_TBL;

    SwSortUndoElement( const String& aS, const String& aT )
    {
        SORT_TXT_TBL.TBL.pSource = new String( aS );
        SORT_TXT_TBL.TBL.pTarget = new String( aT );
    }
    SwSortUndoElement( sal_uLong nS, sal_uLong nT )
    {
        SORT_TXT_TBL.TXT.nSource = nS;
        SORT_TXT_TBL.TXT.nTarget = nT;
        SORT_TXT_TBL.TXT.nKenn   = 0xffffffff;
    }
    ~SwSortUndoElement();
};

typedef boost::ptr_vector<SwSortUndoElement> SwSortList;
typedef std::vector<SwNodeIndex*> SwUndoSortList;

class SwUndoSort : public SwUndo, private SwUndRng
{
    SwSortOptions*  pSortOpt;
    SwSortList      aSortList;
    SwUndoAttrTbl*  pUndoTblAttr;
    SwRedlineData*  pRedlData;
    sal_uLong           nTblNd;

public:
    SwUndoSort( const SwPaM&, const SwSortOptions& );
    SwUndoSort( sal_uLong nStt, sal_uLong nEnd, const SwTableNode&,
                const SwSortOptions&, bool bSaveTable );

    virtual ~SwUndoSort();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void Insert( const String& rOrgPos, const String& rNewPos );
    void Insert( sal_uLong nOrgPos, sal_uLong nNewPos );
};

#endif // SW_UNDO_SORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
