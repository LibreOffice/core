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

#ifndef INCLUDED_SC_SOURCE_UI_INC_EDITABLE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_EDITABLE_HXX

#include "address.hxx"

class ScDocument;
class ScViewFunc;
class ScMarkData;
class ScRange;

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
            ScEditableTester( ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

            // calls TestSelectedBlock
            ScEditableTester( ScDocument* pDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );

            // calls TestRange
            ScEditableTester( ScDocument* pDoc, const ScRange& rRange );

            // calls TestSelection
            ScEditableTester( ScDocument* pDoc, const ScMarkData& rMark );

            // calls TestView
            ScEditableTester( ScViewFunc* pView );

    ScEditableTester(
        const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd,
        const ScMarkData& rMark );

            // Several calls to the Test... methods check if *all* of the ranges
            // are editable. For several independent checks, Reset() has to be used.
    void    TestBlock( ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void    TestSelectedBlock( ScDocument* pDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );
    void    TestRange( ScDocument* pDoc, const ScRange& rRange );
    void    TestSelection( ScDocument* pDoc, const ScMarkData& rMark );

    void TestBlockForAction(
        const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd,
        const ScMarkData& rMark );

    bool IsEditable() const { return mbIsEditable; }
    bool IsFormatEditable() const { return mbIsEditable || mbOnlyMatrix; }
    const char* GetMessageId() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
