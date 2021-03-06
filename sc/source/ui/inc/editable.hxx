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

class ScDocument;
class ScViewFunc;
class ScMarkData;

namespace sc {

enum class ColRowEditAction;

}

class ScEditableTester
{
    bool mbIsEditable;
    bool mbOnlyMatrix;

public:
            ScEditableTester();

            // calls TestBlock
            /** @param  bNoMatrixAtAll
                        TRUE if there must not be any matrix, not even entirely
                        contained; for example in sorting. */
            ScEditableTester( const ScDocument& rDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        bool bNoMatrixAtAll = false );

            // calls TestSelectedBlock
            ScEditableTester( const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );

            // calls TestRange
            ScEditableTester( const ScDocument& rDoc, const ScRange& rRange );

            // calls TestSelection
            ScEditableTester( const ScDocument& rDoc, const ScMarkData& rMark );

            // calls TestView
            ScEditableTester( ScViewFunc* pView );

    ScEditableTester(
        const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd,
        const ScMarkData& rMark );

            // Several calls to the Test... methods check if *all* of the ranges
            // are editable. For several independent checks, Reset() has to be used.
    void    TestBlock( const ScDocument& rDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        bool bNoMatrixAtAll = false );
    void    TestSelectedBlock( const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );
    void    TestRange( const ScDocument& rDoc, const ScRange& rRange );
    void    TestSelection( const ScDocument& rDoc, const ScMarkData& rMark );

    void TestBlockForAction(
        const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd,
        const ScMarkData& rMark );

    bool IsEditable() const { return mbIsEditable; }
    bool IsFormatEditable() const { return mbIsEditable || mbOnlyMatrix; }
    const char* GetMessageId() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
