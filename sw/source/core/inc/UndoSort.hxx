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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOSORT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOSORT_HXX

#include <undobj.hxx>

#include <rtl/ustring.hxx>
#include <tools/solar.h>

#include <memory>
#include <vector>

struct SwSortOptions;
class SwTableNode;
class SwUndoAttrTable;

struct SwSortUndoElement
{
    OUString maSourceString;
    OUString maTargetString;
    SwNodeOffset mnSourceNodeOffset;
    SwNodeOffset mnTargetNodeOffset;

    SwSortUndoElement(const OUString& aSource, const OUString& aTarget)
        : maSourceString(aSource)
        , maTargetString(aTarget)
    {}

    SwSortUndoElement(SwNodeOffset nSource, SwNodeOffset nTarget)
        : mnSourceNodeOffset(nSource)
        , mnTargetNodeOffset(nTarget)
    {}
};

class SwUndoSort final : public SwUndo, private SwUndRng
{
    std::unique_ptr<SwSortOptions>    m_pSortOptions;
    std::vector<std::unique_ptr<SwSortUndoElement>> m_SortList;
    std::unique_ptr<SwUndoAttrTable>  m_pUndoAttrTable;
    SwNodeOffset         m_nTableNode;

public:    SwUndoSort( const SwPaM&, const SwSortOptions& );
    SwUndoSort( SwNodeOffset nStt, SwNodeOffset nEnd, const SwTableNode&,
                const SwSortOptions&, bool bSaveTable );

    virtual ~SwUndoSort() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void Insert( const OUString& rOrgPos, const OUString& rNewPos );
    void Insert( SwNodeOffset nOrgPos, SwNodeOffset nNewPos );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
