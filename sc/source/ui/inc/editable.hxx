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

#pragma once

#include <address.hxx>
#include <unotools/resmgr.hxx>

class ScDocument;
class ScViewFunc;
class ScMarkData;

namespace sc {
enum class EditAction;
}

class ScEditableTester
{
private:
    bool mbIsEditable = true;
    bool mbOnlyMatrix = true;

public:
    ScEditableTester();

    // Enable move constructor
    ScEditableTester(ScEditableTester&& rOther) noexcept = default;

    // Allow copy assignment
    ScEditableTester& operator=(const ScEditableTester& rOther) = default;

    /** @param  bNoMatrixAtAll
                TRUE if there must not be any matrix, not even entirely
                contained; for example in sorting. */
    static ScEditableTester CreateAndTestBlock(const ScDocument& rDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        bool bNoMatrixAtAll = false);

    static ScEditableTester CreateAndTestSelectedBlock(const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark);

    static ScEditableTester CreateAndTestRange(const ScDocument& rDoc, const ScRange& rRange, sc::EditAction eAction);

    static ScEditableTester CreateAndTestSelection(const ScDocument& rDoc, const ScMarkData& rMark);

    static ScEditableTester CreateAndTestView(ScViewFunc* pView);

    static ScEditableTester CreateAndTestBlockForAction(
                const ScDocument& rDoc, sc::EditAction eAction, SCCOL nStartCol,
                SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScMarkData& rMark);

            // Several calls to the Test... methods check if *all* of the ranges
            // are editable. For several independent checks, Reset() has to be used.
    void    TestBlock( const ScDocument& rDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        bool bNoMatrixAtAll = false );
    void    TestBlockForAction( const ScDocument& rDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        sc::EditAction eAction );
    void    TestSelectedBlock( const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );
    void    TestRange( const ScDocument& rDoc, const ScRange& rRange );
    void    TestRangeForAction( const ScDocument& rDoc, const ScRange& rRange,
                       sc::EditAction eAction );
    void    TestSelection( const ScDocument& rDoc, const ScMarkData& rMark );

    void    TestBlockForAction(
                        const ScDocument& rDoc, sc::EditAction eAction, SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, const ScMarkData& rMark );

    bool IsEditable() const { return mbIsEditable; }
    bool IsFormatEditable() const { return mbIsEditable || mbOnlyMatrix; }
    TranslateId GetMessageId() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
